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
#include <lanxc-applism/config.hpp>

#include <lanxc-unixy/unixy.hpp>

namespace lanxc
{
  namespace applism
  {

    class event_service;

    class LANXC_APPLISM_EXPORT event_source
    {
    public:
      virtual const unixy::file_descriptor &get_file_descriptor() const noexcept = 0;

      virtual ~event_source() = 0;
    };


    class LANXC_APPLISM_EXPORT concrete_event_source : public virtual event_source
    {
    public:

      concrete_event_source(unixy::file_descriptor fd) noexcept;

      ~concrete_event_source() = default;

      const unixy::file_descriptor &get_file_descriptor() const noexcept override
      {
        return _file_descriptor;
      }
    private:
      unixy::file_descriptor _file_descriptor;
    };
  }
}
