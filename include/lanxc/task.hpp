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

#ifndef LANXC_TASK_SERVICE_HPP_INCLUDED
#define LANXC_TASK_SERVICE_HPP_INCLUDED

#include <lanxc/function.hpp>
#include <lanxc/link.hpp>


#include <chrono>
#include <memory>

namespace lanxc
{
  class task;
  class task_token;
  class scheduler;

  using std::chrono::steady_clock;
  using std::chrono::system_clock;
  using time_duration = steady_clock::duration;

  class task_listener : public link::list_node<task_listener>
  {
    friend class scheduler;
  protected:

    /**
     * @brief Destructor of a task
     */
    virtual ~task_listener() = default;


    /**
     * @brief Function that called when a task is finished
     */
    virtual void on_finish() = 0;
  };

  class task_progress_listener
  {
    friend class scheduler;
  protected:
    virtual ~task_progress_listener() = default;

    /**
     * @brief Function that called when associated task updates its progress
     * @param current A number indicates the current progress of a task
     * @param total A number indicates the total progress of a task
     */
    virtual void on_progress_changed(unsigned current, unsigned total) = 0;

  private:
    unsigned m_current_progress;
    unsigned m_total_progress;
  };


  class task_token
  {
    friend class scheduler;
  public:

    constexpr task_token () noexcept
        : m_scheduler{nullptr}
        , m_listener{nullptr}
    { }

    task_token(const task_token &tm) = delete;
    task_token &operator = (const task_token &tm) =delete;

    task_token(task_token &&tm) noexcept ;
    task_token &operator = (task_token &&tm) noexcept ;

    void set_progress(unsigned current, unsigned total);
    ~task_token();

    scheduler &get_scheduler() const noexcept
    { return *m_scheduler; }

  private:
    task_token(scheduler *s, task_listener *t) noexcept ;
    scheduler *m_scheduler;
    task_listener *m_listener;
  };

  class task : public task_listener
             , public link::list_node<task>
  {
    friend class scheduler;
  protected:

    virtual ~task() = default;
    /**
     * @brief The routine of this task
     * @param tm The task monitor for this routine to update progress
     * @note This function should not throws any exception, any exception
     * should be caught and handled in its own way
     */
    virtual void routine(task_token tm) noexcept = 0;
  };

  /**
   * @brief Task that run at specified time
   */
  class schedule
    : public link::rbtree_node<steady_clock::time_point, schedule>
    , public task
  {
    friend class scheduler;
  protected:

    /**
     * @brief Construct a schedule initially runs at given time, repeat in
     * specified duration
     * @param tp The initial time
     * @param td The duration, 0 means no repeat, default is 0
     */
    schedule(steady_clock::time_point tp, time_duration td);

    /**
     * @brief Construct a schedule runs repeatedly
     * @param td The duration between each run
     */
    schedule(time_duration td);

  private:
    time_duration m_duration;
  };



  /**
   * @brief This class schedules tasks
   *
   * The implement of this class should run the scheduled task in any way,
   * e.g. a background thread or defer until event loop is free, and finally
   * notify that if the task finished or failed
   */
  class scheduler
  {
    friend class task_token;
  public:

    /**
     * @brief Schedule a task to run
     * @note This function can be called from any thread
     * @param t The task
     */
    virtual void schedule(task &t) = 0;

    /**
     * @brief Start to run the scheduled tasks
     * @note This function should only be called from the running thread of
     * this scheduler
     * @param t The task
     */
    virtual void dispatch(task &t) = 0;

    virtual void start() = 0;


    virtual ~scheduler() = default;

  protected:

    /**
     * @brief Executing a task
     * @param t The task
     * @param l The listener
     */
    void execute(task &t);

    /**
     * @brief Run the function task_listener::on_finished of a task listener
     * @param l The task listener
     */
    void do_notify_finished(task_listener &l);

    /**
     * @brief Notify that a task is finished
     * @param t The task
     */
    virtual void notify_finished(task_listener &t) = 0;

  };



  class thread_pool_scheduler : public scheduler
  {
  public:
    thread_pool_scheduler();

    ~thread_pool_scheduler();

    void schedule(task &t) override;

    void dispatch(task &t) override;

    void start() override;
  protected:

    virtual void notify_finished(task_listener &r) override;

  private:
    struct detail;
    std::unique_ptr<detail> m_detail;
  };
}



#endif //LANXC_CORE_TASK_HPP
