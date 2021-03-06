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
#include <lanxc-applism/event_service.hpp>
#include <lanxc-applism/config.hpp>

#include <lanxc/core/task_context.hpp>
#include <lanxc/core/io_context.hpp>
#include <lanxc/core/network_context.hpp>

#include <memory>
#include <chrono>

namespace lanxc
{
  namespace applism
  {
    class LANXC_APPLISM_EXPORT event_loop
        : public virtual task_context
        , public virtual io_context
        , public virtual event_service
    {
    public:

      event_loop();

      ~event_loop();

      void run() override;

      void add_event(const unixy::file_descriptor &fd,
                     readable_event_channel &channel) override;

      void add_event(const unixy::file_descriptor &fd,
                     writable_event_channel &channel) override;

      std::shared_ptr<deferred> defer(function<void()> routine) override;

      std::shared_ptr<alarm> schedule(time_point t,
                                      function<void()> routine) override;

    private:
      struct detail;
      std::shared_ptr<detail>  _detail;
    };
  }

}
