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
#include <cassert>
#include <iostream>

namespace
{
  bool executed = false;
  bool finished = false;
  bool failed = false;

  struct my_task : lanxc::task
  {
    virtual ~my_task() =default;

  protected:
    virtual void on_finish() override
    {
      finished = true;
    }

    virtual void routine(lanxc::task_monitor tm) noexcept override
    {
      executed = true;
      tm.set_progress(0,1);
      tm.set_progress(2,2);
    }

    virtual void on_progress_changed(unsigned current, unsigned total) override
    {
      std::cout << current << '/' << total << std::endl;
    }
  };

}

int main()
{

  my_task m; // 10390575
  lanxc::thread_pool_scheduler scheduler;
  scheduler.schedule(m);
  scheduler.start();
  assert(executed);
  assert(finished);
  assert(!failed);
}
