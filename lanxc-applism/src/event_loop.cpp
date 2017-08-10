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

#include <lanxc-applism/event_service.hpp>
#include <lanxc-applism/event_loop.hpp>
#include <lanxc-unixy/unixy.hpp>

#include <system_error>
#include <mutex>
#include <vector>
#include <array>

#include <sys/event.h>
#include <unistd.h>

namespace
{
  int create_kqueue()
  {
    int ret = ::kqueue();
    if (ret == -1) lanxc::unixy::throw_system_error();
    return ret;
  }
}

namespace lanxc
{
  namespace applism
  {
    struct event_loop::detail : event_channel
                              , event_service
                              , concrete_event_source
    {

      std::mutex _mutex;
      std::vector<struct kevent> _changed_events_list;


    public:

      detail()
        : concrete_event_source(unixy::file_descriptor{create_kqueue()})
      {
      }


      void signal(std::intptr_t, std::uint32_t) override
      { /* Just empty */ }

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

        int ret = kevent(get_file_descriptor(), nullptr, 0, &ke, 1, nullptr);
        if (ret == 0) return;
        lanxc::unixy::throw_system_error();
      }

      void pool()
      {
        while (true)
        {
          std::array<struct kevent, 256> events;

          int ret = ::kevent(get_file_descriptor(),
                             _changed_events_list.data(),
                             static_cast<int>(_changed_events_list.size()),
                             events.data(),
                             static_cast<int>(events.size()),
                             nullptr);

          if (ret < 0)
            lanxc::unixy::throw_system_error();

          _changed_events_list.clear();

          if (ret == 0)
            continue;

          for (int i = 0; i < ret; i++)
          {
            auto *p = reinterpret_cast<event_channel *>(events[i].udata);
            p->signal(events[i].data, events[i].fflags);
          }
        }
      }

      void enable_event_channel(int16_t event, uint32_t flag, std::intptr_t data, event_channel &channel) override
      {
        int fd = get_file_descriptor();
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

      }

    };

    event_loop::event_loop()
      : _detail { new detail() }
    {}
    event_loop::~event_loop() {}

    void event_loop::run()
    {
      _detail->pool();
    }

    std::shared_ptr<connection_listener_builder>
    event_loop::create_connection_listener()
    {
      return nullptr;
    }

    std::shared_ptr<connection_endpoint_builder>
    event_loop::create_connection_endpoint()
    {
      return nullptr;
    }

    std::shared_ptr<datagram_endpoint_builder>
    event_loop::create_datagram_endpoint()
    {
      return nullptr;
    }

    void event_loop::enable_event_channel(std::int16_t event,
                                          uint32_t flag,
                                          std::intptr_t data,
                                          event_channel &channel)
    {
      _detail->enable_event_channel(event, flag, data, channel);
    }
  }
}
