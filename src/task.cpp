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
#include <mutex>
#include <condition_variable>

namespace lanxc
{

  task_token::task_token(task_token &&tm) noexcept
      : m_scheduler(nullptr)
      , m_listener(nullptr)
  {
    std::swap(m_scheduler, tm.m_scheduler);
    std::swap(m_listener, tm.m_listener);
  }

  task_token &task_token::operator =(task_token &&tm) noexcept
  {
    this->~task_token();
    new (this) task_token(std::move(tm));
    return *this;
  }

  task_token::task_token(scheduler *s, task_listener *l) noexcept
      : m_scheduler(s)
      , m_listener(l)
  { }

  task_token::~task_token()
  {
    if (m_scheduler && m_listener)
    {
      m_scheduler->notify_finished(*m_listener);
      m_scheduler = nullptr;
      m_listener = nullptr;
    }
  }

  void scheduler::execute(task &t)
  {
    t.routine(task_token(this, &t));
  }

  void scheduler::do_notify_finished(task_listener &t)
  {
    t.on_finish();
  }

  struct thread_pool_scheduler::detail
  {
    void routine()
    {
      if (!wait_for_start()) return;
      while(true)
      {
        link::list<task> tmp;
        {
          std::unique_lock<std::mutex> lg(m_mutex);
          if (m_exited)
            return;
          if (m_committed_task.empty())
            m_condition.wait(lg);
          m_committed_task.swap(tmp);
        }
        while(!tmp.empty())
        {
          auto &t = tmp.front();
          tmp.pop_front();
          m_scheduler.execute(t);
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
        : m_scheduler(s)
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


    thread_pool_scheduler     &m_scheduler;
    std::mutex                m_mutex{};
    std::condition_variable   m_condition{};
    std::condition_variable   m_notify_condition{};
    link::list<task>          m_prepared_task;
    link::list<task_listener> m_finished_task;
    link::list<task>          m_committed_task;
    std::list<std::thread>    m_threads{};
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
    m_detail->m_committed_task.push_back(t);
    m_detail->m_condition.notify_one();
  }

  void thread_pool_scheduler::dispatch(task &t)
  {
    m_detail->m_prepared_task.push_back(t);
  }

  void thread_pool_scheduler::notify_finished(task_listener &l)
  {
    std::lock_guard<std::mutex> lg(m_detail->m_mutex);
    m_detail->m_finished_task.push_back(l);
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

    link::list<task_listener> finished_list;
    for ( ; ; )
    {
      if (m_detail->m_exited)
        return;

      if (!finished_list.empty())
      {
        while(!finished_list.empty())
        {
          m_detail->m_task_count --;
          auto &r = finished_list.front();
          finished_list.pop_front();
          do_notify_finished(r);
        }
      }

      std::unique_lock<std::mutex> lg(m_detail->m_mutex);

      m_detail->m_task_count += m_detail->m_prepared_task.size();
      m_detail->m_committed_task.splice(m_detail->m_committed_task.end(),
                                        m_detail->m_prepared_task);

      if (m_detail->m_task_count == 0)
      {
        m_detail->m_exited = true;
        m_detail->m_condition.notify_one();
        return;
      }

      finished_list.swap(m_detail->m_finished_task);
      if (m_detail->m_committed_task.empty())
      {
        while (finished_list.empty())
        {
          m_detail->m_notify_condition.wait(lg);
          finished_list.swap(m_detail->m_finished_task);
        }
      }
      m_detail->m_condition.notify_all();

    }
  }
}