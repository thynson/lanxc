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

#include <lanxc-applism/event_channel.hpp>
#include <lanxc-applism/event_service.hpp>

#include <sys/event.h>
#include "connection.hpp"

namespace lanxc
{
  namespace applism
  {


    readable_event_channel::readable_event_channel(event_service &es)
    {
      es.enable_event_channel(EVFILT_READ, 0, 0, *this);
    }

    writable_event_channel::writable_event_channel(event_service &es)
    {
      es.enable_event_channel(EVFILT_READ, 0, 0, *this);
    }

    error_event_channel::error_event_channel(event_service &es)
    {
      es.enable_event_channel(EVFILT_EXCEPT, 0, 0, *this);
    }

  }
}
