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

  task_monitor::task_monitor(task_monitor &&tm) noexcept
      : m_scheduler(nullptr)
      , m_listener(nullptr)
  {
    std::swap(m_scheduler, tm.m_scheduler);
    std::swap(m_listener, tm.m_listener);
  }

  task_monitor &task_monitor::operator =(task_monitor &&tm) noexcept
  {
    this->~task_monitor();
    new (this) task_monitor(std::move(tm));
    return *this;
  }

  task_monitor::task_monitor(scheduler &s, task_listener &l) noexcept
      : m_scheduler(&s)
      , m_listener(&l)
  { }

  task_monitor::~task_monitor()
  {
    if (m_scheduler && m_listener)
    {
      m_scheduler->notify_finished(*m_listener);
      m_scheduler = nullptr;
      m_listener = nullptr;
    }
  }

  void task_monitor::set_progress(unsigned current, unsigned total)
  {

    if (m_scheduler && m_listener)
      m_scheduler->notify_progress(*m_listener, current, total);
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
    {
      std::unique_lock<std::mutex> lg(m_detail->m_mutex);

      if (!m_detail->m_threads.empty()) return;

      for (auto i = 0U; i < std::thread::hardware_concurrency(); ++i)
        m_detail->m_threads.emplace_back([this] {m_detail->routine();});

    }

    detail::task_notify_queue finish_queue;
    detail::task_progress_table progress_table;
    for ( ; ; )
    {
      if (m_detail->m_exited)
        return;

      if (!finish_queue.empty())
      {
        while(!finish_queue.empty())
        {
          m_detail->m_task_count --;
          auto &r = finish_queue.front().get();
          finish_queue.pop();
          do_notify_finished(r);
        }
      }

      if (!progress_table.empty())
      {
        for (auto &p : progress_table)
        {
          do_notify_progress(p.first.get(), p.second.first, p.second.second);
        }
        progress_table.clear();
      }


      std::unique_lock<std::mutex> lg(m_detail->m_mutex);

      if (m_detail->m_task_count == 0)
      {
        m_detail->m_exited = true;
        m_detail->m_condition.notify_one();
        return;
      }
      while(m_detail->m_finished_queue.empty()
            && m_detail->m_task_progresses.empty())
      {

        m_detail->m_notify_condition.wait(lg);
      }
      finish_queue.swap(m_detail->m_finished_queue);
      progress_table.swap(m_detail->m_task_progresses);
    }
  }
}