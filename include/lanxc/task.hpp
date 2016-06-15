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

#ifndef LANXC_TASK_HPP_INCLUDED
#define LANXC_TASK_HPP_INCLUDED

#include <memory>
#include "function.hpp"

namespace lanxc
{
  class task;
  class task_monitor;
  class scheduler;

  class task_listener
  {
    friend class scheduler;
  protected:

    /**
     * @brief Destructor of a task
     */
    virtual ~task_listener() = default;

    /**
     * @brief Function that called when associated task updates its progress
     * @param current A number indicates the current progress of a task
     * @param total A number indicates the total progress of a task
     */
    virtual void on_progress_changed(unsigned current, unsigned total) = 0;

    /**
     * @brief Function that called when a task is finished
     */
    virtual void on_finish() = 0;
  };


  class task_monitor
  {
    friend class scheduler;
  public:

    constexpr task_monitor () noexcept
        : m_scheduler{nullptr}
        , m_listener{nullptr}
    { }

    task_monitor(const task_monitor &tm) = delete;
    task_monitor &operator = (const task_monitor &tm) =delete;

    task_monitor(task_monitor &&tm) noexcept ;
    task_monitor &operator = (task_monitor &&tm) noexcept ;

    void set_progress(unsigned current, unsigned total);
    ~task_monitor();

    scheduler &get_scheduler() const noexcept
    { return *m_scheduler; }

  private:
    task_monitor(scheduler *s, task_listener *t) noexcept ;
    scheduler *m_scheduler;
    task_listener *m_listener;
  };

  class task : public task_listener
  {
    friend class scheduler;
  protected:

    /**
     * @brief Destructor of a task
     */
    virtual ~task() = default;

    /**
     * @brief The routine of this task
     * @param tm The task monitor for this routine to update progress
     * @note This function should not throws any exception, any exception
     * should be caught and handled in its own way
     */
    virtual void routine(task_monitor tm) noexcept = 0;
  };


  /**
   * @brief This class schedules tasks
   * @note This class is concret through but actually design as an abstract
   * class. The default implementation of this class is naive that runs
   * scheduled tasks immediately.
   *
   * The implement of this class should run the scheduled task in any way,
   * e.g. a background thread or defer until event loop is free, and finally
   * notify that if the task finished or failed
   */
  class scheduler
  {
    friend class task_monitor;
  public:

    /**
     * @brief Schedule a task to run
     * @param t The task
     */
    virtual void schedule(task &t) = 0;

    /**
     * @brief Start to run the scheduled tasks
     */
    virtual void start();

    virtual ~scheduler() = default;

  protected:

    /**
     * @brief Executing a task
     * @param t The task
     * @param l The listener
     */
    void execute(task &t);

    /**
     * @brief Run the function task_listener::on_progress_changed of a task listener
     * @param l The task listener
     * @param current The number indicates current progress
     * @param total The number indicates total progress
     */
    void do_notify_progress(task_listener &l, unsigned current, unsigned total);

    /**
     * @brief Run the function task_listener::on_finished of a task listener
     * @param l The task listener
     */
    void do_notify_finished(task_listener &l);

    /**
     * @brief Notify that the progress of a task has updated
     * @param l The task listener
     * @param current A number indicates current progress
     * @param total A number indicate total progress
     */
    virtual void notify_progress(task_listener &l, unsigned current, unsigned total) = 0;

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

    void start() override;
  protected:

    virtual void notify_progress(task_listener &t, unsigned current, unsigned total) override;

    virtual void notify_finished(task_listener &r) override;

  private:
    struct detail;
    std::unique_ptr<detail> m_detail;
  };
}



#endif //LANXC_CORE_TASK_HPP
