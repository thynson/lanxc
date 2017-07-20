/*
 * Copyright (C) 2017 LAN Xingcan
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

#pragma once

#include <lanxc/function.hpp>
#include <lanxc/config.hpp>

namespace lanxc
{


  class executor_context
  {
  public:
    virtual void dispatch(lanxc::function<void()> routine) = 0;

    virtual void run() = 0;
  };

  class io_event
  {

  };

  class io_proactor
  {
  public:
    virtual void signal() = 0;
  protected:
    virtual size_t poll(bool block) = 0;

  };

  class event_loop : public virtual main_loop
                   , private virtual io_proactor
  {
  public:

    virtual void run() override
    {
      while (true)
      {
        auto count = process_tasks();
        if (count == 0)
        {
          if (poll(true) == 0)
            return;
        }
        else
          poll(false);
      }
    }
  private:

  };

}


