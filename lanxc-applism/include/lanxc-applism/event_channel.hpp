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

#include <lanxc-applism/event_source.hpp>
#include <lanxc-applism/config.hpp>

#include <lanxc-unixy/unixy.hpp>

#include <cstdint>

namespace lanxc
{
  namespace applism
  {


    class LANXC_APPLISM_EXPORT event_channel : protected virtual event_source
    {
      friend class event_service;
    public:

      event_channel() = default;
      virtual ~event_channel() = default;

      event_channel(const event_channel &) = delete;
      event_channel(event_channel &&) = delete;
      event_channel &operator = (const event_channel &) = delete;
      event_channel &operator = (event_channel &&) = delete;

      virtual void signal(std::intptr_t data, std::uint32_t flags) = 0;
    };

  }
}

