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
#include <lanxc/core-macos/event_service.hpp>

#include <lanxc/core/task_context.hpp>
#include <lanxc/core/io_context.hpp>
#include <lanxc/core/network_context.hpp>

#include <memory>

namespace lanxc
{
  namespace macos
  {
    class event_loop : public virtual task_context
                     , public virtual io_context
                     , public virtual network_context
                     , public virtual event_service
    {
    public:

      event_loop();
      ~event_loop() override;
      void start() override;

      std::shared_ptr<connection_listener_builder>
      create_connection_listener() override;

      std::shared_ptr<connection_endpoint_builder>
      create_connection_endpoint() override;

      std::shared_ptr<datagram_endpoint_builder>
      create_datagram_endpoint() override;

      void register_event(int descriptor,
                          int16_t event,
                          uint16_t operation,
                          uint32_t flag,
                          std::intptr_t data,
                          event_channel &channel) override;
    private:
      struct detail;
      std::unique_ptr<detail>  _detail;
    };
  }

}
