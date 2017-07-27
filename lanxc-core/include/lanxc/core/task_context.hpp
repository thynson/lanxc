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

#pragma once

#include <lanxc/function.hpp>
#include <lanxc/config.hpp>

#include <memory>

namespace lanxc
{
  class task_context;

  class LANXC_CORE_EXPORT deferred
  {
    friend class task_context;
  public:
    virtual ~deferred() = 0;
    virtual void cancel() = 0;
  private:
    virtual void execute() = 0;
  };

  class LANXC_CORE_EXPORT alarm : public deferred
  {
  public:
    virtual void schedule() = 0;
    virtual ~alarm() = 0;
  };

  class LANXC_CORE_EXPORT task_context
  {
  public:

    virtual ~task_context() = 0;

    virtual std::shared_ptr<deferred>
    defer(function<void()> routine) = 0;

    virtual std::shared_ptr<alarm>
    schedule(std::uint64_t useconds, function<void()> routine) = 0;

    virtual void run() = 0;

  protected:
    virtual std::size_t process_tasks() = 0;
  };
}
