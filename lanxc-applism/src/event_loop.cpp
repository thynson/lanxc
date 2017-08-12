/*
 * Copyright (C) 2016 LAN Xingcan
 * All right reserved
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include <sys/event.h>
#include <unistd.h>

#include <system_error>
#include <mutex>
#include <vector>
#include <array>
#include <chrono>

#include <lanxc-unixy/unixy.hpp>

#include <lanxc/link.hpp>

#include <lanxc-applism/event_service.hpp>
#include <lanxc-applism/event_loop.hpp>
#include <lanxc-applism/event_channel.hpp>


namespace lanxc
{
  namespace link
  {
    template<>
    struct rbtree_config<applism::event_loop> : rbtree_config<void>
    {
      using default_lookup_policy = index_policy::back;
    };

  }
}

namespace
{
  using namespace lanxc;
  int create_kqueue()
  {
    int ret = ::kqueue();
    if (ret == -1) unixy::throw_system_error();
    return ret;
  }

  struct event_loop_task
      : virtual lanxc::deferred
      , link::list_node<event_loop_task, void>
  {
    function<void()> _routine;

    event_loop_task(function<void()> r) noexcept
        : _routine(std::move(r))
    { }

    ~event_loop_task() = default;

    event_loop_task(const event_loop_task &) = delete;
    event_loop_task(event_loop_task &&) = delete;
    event_loop_task &operator = (const event_loop_task &) = delete;
    event_loop_task &operator = (event_loop_task &&) = delete;

    void cancel() override
    {
      unlink();
    }

    void execute() override
    {
      _routine();
    }
  };

  using alarm_clock_type = std::chrono::steady_clock::time_point;
  template<typename Alarm>
  using event_loop_rbtree_node = link::rbtree_node<alarm_clock_type,
                                                   Alarm,
                                                   applism::event_loop>;


  struct event_loop_alarm
      : virtual alarm
      , event_loop_task
      , event_loop_rbtree_node<event_loop_alarm>
  {
    using rbtree_node = event_loop_rbtree_node<event_loop_alarm>;
    event_loop_alarm(alarm_clock_type t, function<void()> r) noexcept
        : event_loop_task {std::move(r)}
        , rbtree_node {std::move(t)}
    { }

    event_loop_alarm(const event_loop_alarm &) = delete;
    event_loop_alarm(event_loop_alarm &&) = delete;
    event_loop_alarm &operator = (const event_loop_alarm &) = delete;
    event_loop_alarm &operator = (event_loop_alarm &&) = delete;

    void cancel() override
    {
      event_loop_task::cancel();
      rbtree_node::unlink();
    }

    ~event_loop_alarm() = default;

  };
}

namespace lanxc
{
  namespace applism
  {
    struct event_loop::detail
    {

      std::mutex _mutex;
      unixy::file_descriptor _kqueue_fd;
      std::vector<struct kevent> _changed_events_list;
      link::list<event_channel> _enabled_event_channels;
      link::list<event_loop_task> _deferred_tasks;
      link::rbtree<alarm_clock_type, event_loop_alarm, event_loop> _scheduled_alarms;


    public:

      void add_event(int fd, readable_event_channel &channel)
      {
        add_event_channel(fd, EVFILT_READ, 0, 0, channel);
      }

      void add_event(int fd, writable_event_channel &channel)
      {
        add_event_channel(fd, EVFILT_WRITE, 0, 0, channel);
      }

      detail()
        : _kqueue_fd(unixy::file_descriptor{create_kqueue()})
      {
      }

      void activate()
      {

        struct kevent ke;

        EV_SET(&ke,
               reinterpret_cast<uintptr_t>(this),
               EVFILT_TIMER,
               EV_ADD|EV_ONESHOT,
               NOTE_SECONDS,
               0,
               this);

        int ret = kevent(_kqueue_fd, nullptr, 0, &ke, 1, nullptr);
        if (ret == 0) return;
        lanxc::unixy::throw_system_error();
      }

      void process_alarms(const time_point &now)
      {
        auto upper_bound = _scheduled_alarms.upper_bound(now);
        auto i = _scheduled_alarms.begin();
        while (i != upper_bound)
        {
          auto &t = *i++;
          _deferred_tasks.push_back(t);
        }
      }

      void process_tasks()
      {
        if (!_deferred_tasks.empty())
        {
          auto tasks = std::move(_deferred_tasks);
          for (auto &t : tasks)
          {
            t.execute();
          }
        }
      }

      timespec *decide_waiting_duration(time_point now, timespec &ts)
      {

        if (!_deferred_tasks.empty())
        {
          ts.tv_sec = 0;
          ts.tv_nsec = 0;
          return &ts;
        }
        else if (!_scheduled_alarms.empty())
        {
          auto &t = _scheduled_alarms.front().get_index();
          auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(t - now);
          auto secs = std::chrono::duration_cast<std::chrono::seconds>(nanos);
          nanos -= secs;
          ts.tv_sec = secs.count();
          ts.tv_nsec = nanos.count();
          return &ts;
        }
        return nullptr;
      }

      void poll()
      {
        while (true)
        {
          auto now = std::chrono::steady_clock::now();
          process_alarms(now);
          process_tasks();
          timespec ts;
          auto pt = decide_waiting_duration(now, ts);
          if (pt == nullptr && _enabled_event_channels.empty())
            return ;

          std::array<struct kevent, 256> events;

          int ret = ::kevent(_kqueue_fd,
                             _changed_events_list.data(),
                             static_cast<int>(_changed_events_list.size()),
                             events.data(),
                             static_cast<int>(events.size()),
                             pt);

          if (ret < 0)
            lanxc::unixy::throw_system_error();

          _changed_events_list.clear();

          if (ret == 0)
            continue;

          for (int i = 0; i < ret; i++)
          {
            auto *p = reinterpret_cast<event_channel *>(events[i].udata);

            switch(events[i].filter)
            {
            case EVFILT_READ:
              {
                auto *rp = static_cast<readable_event_channel*>(p);

                if (events[i].flags & EV_ERROR)
                {
                  auto e = static_cast<std::uint32_t>(events[i].data);
                  rp->on_reading_error(e);
                }
                else if (events[i].flags & EV_EOF)
                {
                  auto e = events[i].fflags;
                  rp->on_reading_error(e);
                }
                else
                {
                  rp->on_readable(ssize_t(events[i].data));
                }
              }

              break;
            case EVFILT_WRITE:
              {
                auto *wp = static_cast<writable_event_channel*>(p);

                if (events[i].flags & EV_ERROR)
                {
                  auto e = static_cast<std::uint32_t>(events[i].data);
                  wp->on_writing_error(e);
                }
                else if (events[i].flags & EV_EOF)
                {
                  auto e = events[i].fflags;
                  wp->on_writing_error(e);
                }
                else
                {
                  wp->on_writable(ssize_t(events[i].data));
                }

              }

              break;

            default:
              break;

            }
          }
        }
      }

      void add_event_channel(int fd, int16_t event, uint32_t flag,
                             std::intptr_t data,
                             event_channel &channel)
      {
        _changed_events_list.push_back(
                   {
                       static_cast<std::uintptr_t>(fd),
                       event,
                       EV_ADD | EV_CLEAR,
                       flag,
                       data,
                       &channel
                   }
               );
        _enabled_event_channels.push_back(channel);

      }

    };

    event_loop::event_loop()
      : _detail { std::make_shared<detail>() }
    {}

    event_loop::~event_loop() {}

    void event_loop::run()
    {
      _detail->poll();
    }

    void event_loop::add_event(const unixy::file_descriptor &fd,
                               readable_event_channel &channel)
    {
      _detail->add_event(fd, channel);
    }

    void event_loop::add_event(const unixy::file_descriptor &fd,
                               writable_event_channel &channel)
    {
      _detail->add_event(fd, channel);
    }

    std::shared_ptr<deferred> event_loop::defer(function<void()> routine)
    {
      auto p = std::make_shared<event_loop_task>(std::move(routine));
      _detail->_deferred_tasks.push_back(*p);
      return p;
    }

    std::shared_ptr<alarm>
    event_loop::schedule(time_point t,
                         function<void()> routine)
    {

      auto p = std::make_shared<event_loop_alarm>(std::move(t),
                                                  std::move(routine));
      _detail->_scheduled_alarms.insert(*p);
      return p;
    }

  }
}
