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

#include <lanxc/task.hpp>

#include <queue>
#include <thread>
#include <map>
#include <list>

namespace lanxc
{

  task_listener::~task_listener() = default;

  struct task_monitor::detail
  {
    detail(scheduler &s, task_listener &l)
        : m_scheduler(s)
        , m_task_listener(l)
    { }
    scheduler &m_scheduler;
    task_listener &m_task_listener;
  };

  task_monitor::task_monitor(scheduler &s, task_listener &l)
      : m_detail { new detail { s, l } }
  { }

  task_monitor::~task_monitor()
  {
    m_detail->m_scheduler.notify_finished(m_detail->m_task_listener);
  }

  void task_monitor::set_progress(unsigned current, unsigned total)
  {
    m_detail->m_scheduler.notify_progress(m_detail->m_task_listener,
                                             current, total);
  }

  scheduler::~scheduler() = default;

  void scheduler::execute(task &t)
  {
    t.routine(task_monitor(*this, t));
  }

  void scheduler::do_notify_progress(task_listener &t,
                                     unsigned current, unsigned total)
  {
    t.on_progress_changed(current, total);
  }

  void scheduler::do_notify_finished(task_listener &t)
  {
    t.on_finish();
  }

  void scheduler::start() // Noop
  { }

  task::~task() = default;

  struct thread_pool_scheduler::detail
  {
    void routine()
    {
      if (!wait_for_start()) return;
      while(true)
      {
        task_queue tmp;
        {
          std::unique_lock<std::mutex> lg(m_mutex);
          if (m_exited)
            return;
          if (m_queue.empty())
            m_condition.wait(lg);
          m_queue.swap(tmp);
        }
        while(!tmp.empty())
        {
          auto &t = tmp.front();
          tmp.pop();
          m_scheduler.execute(t.get());
        }
      }
    }

    bool wait_for_start()
    {
      std::unique_lock<std::mutex> lg(m_mutex);
      if (m_exited) return false;
      m_notify_condition.notify_one();
      return true;
    }

    detail(thread_pool_scheduler &s)
        : m_scheduler { s }
    { }

    ~detail()
    {
      {
        std::unique_lock<std::mutex> guard(m_mutex);
        if (!m_threads.empty())
        {
          m_exited = true;
          m_condition.notify_all();
        }
      }
      for (auto &t : m_threads) t.join();
    }

    struct reference_comparator
    {
      bool operator () (const task_listener &lhs, const task_listener &rhs)
      const noexcept
      { return &lhs < &rhs; }
    };

    using task_progress_table
      = std::map<std::reference_wrapper<task_listener>,
        std::pair<unsigned, unsigned>, reference_comparator>;

    using task_queue
      = std::queue<std::reference_wrapper<task>>;

    using task_notify_queue
      = std::queue<std::reference_wrapper<task_listener>>;

    thread_pool_scheduler      &m_scheduler;
    std::mutex                m_mutex{};
    std::condition_variable   m_condition{};
    std::condition_variable   m_notify_condition{};
    task_queue                m_queue{};
    task_notify_queue         m_finished_queue{};
    task_progress_table       m_task_progresses {};
    std::list<std::thread>    m_threads;
    std::size_t               m_task_count {0};
    bool                      m_exited {false};
  };

  thread_pool_scheduler::thread_pool_scheduler()
    : m_detail { new detail(*this) }
  { }

  thread_pool_scheduler::~thread_pool_scheduler() = default;

  void thread_pool_scheduler::schedule(task &t)
  {
    std::lock_guard<std::mutex> lg(m_detail->m_mutex);
    m_detail->m_task_count++;
    m_detail->m_queue.emplace(std::ref(t));
    m_detail->m_condition.notify_one();
  }

  void thread_pool_scheduler::notify_progress(task_listener &l, unsigned current, unsigned total)
  {
    std::lock_guard<std::mutex> lg(m_detail->m_mutex);
    auto i = m_detail->m_task_progresses.find(std::ref(l));
    auto v = std::make_pair(current, total);
    if (i == m_detail->m_task_progresses.end())
      m_detail->m_task_progresses.emplace(std::ref(l), std::move(v));
    else
      std::swap(v, i->second);
    m_detail->m_notify_condition.notify_one();
  }

  void thread_pool_scheduler::notify_finished(task_listener &l)
  {
    std::lock_guard<std::mutex> lg(m_detail->m_mutex);
    m_detail->m_finished_queue.emplace(l);
    m_detail->m_notify_condition.notify_one();
  }

  void thread_pool_scheduler::start()
  {
    std::unique_lock<std::mutex> lg(m_detail->m_mutex);

    if (!m_detail->m_threads.empty()) return;

    for (auto i = 0U; i < std::thread::hardware_concurrency(); ++i)
      m_detail->m_threads.emplace_back([this] {m_detail->routine();});

    for ( ; ; )
    {
      bool wait = true;
      if (m_detail->m_exited)
        return;

      if (!m_detail->m_finished_queue.empty())
      {
        detail::task_notify_queue q { std::move(m_detail->m_finished_queue) };
        m_detail->m_task_count -= q.size();
        while(!q.empty())
        {

          auto &r = q.front().get();
          q.pop();
          do_notify_finished(r);
        }
        wait = false;
      }

      if (!m_detail->m_task_progresses.empty())
      {
        detail::task_progress_table tbl { std::move(m_detail->m_task_progresses) };
        for (auto &p : tbl)
        {
          do_notify_progress(p.first.get(), p.second.first, p.second.second);
        }
        wait = false;
      }

      if (m_detail->m_task_count == 0)
      {
        m_detail->m_exited = true;
        m_detail->m_condition.notify_one();
        return;
      }
      if (wait)
        m_detail->m_notify_condition.wait(lg);
    }
  }
}