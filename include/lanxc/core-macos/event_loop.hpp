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

#ifndef LANXC_EVENT_LOOP_HPP
#define LANXC_EVENT_LOOP_HPP

#include <lanxc/core/main_loop.hpp>
#include <lanxc/core/io_context.hpp>


namespace lanxc
{

  class macos_event_loop : public virtual main_loop
                         , public virtual io_context
  {
  public:
    ~macos_event_loop() override;

    void start() override;
  };
}

#endif //LANXC_EVENT_LOOP_HPP
