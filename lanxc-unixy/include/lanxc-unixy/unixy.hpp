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

#include <lanxc/type_traits.hpp>
#include <lanxc-unixy/config.hpp>

#include <utility>
#include <new>

namespace lanxc
{
  namespace unixy
  {
    [[noreturn]] void LANXC_UNIXY_EXPORT
    throw_system_error();

    [[noreturn]] void LANXC_UNIXY_EXPORT
    throw_system_error(int e);




    class LANXC_UNIXY_EXPORT file_descriptor
    {
    public:
      constexpr file_descriptor(int fd = -1) noexcept
          : _fd(fd)
      {}
      template<typename F, typename ...Arguments>
      file_descriptor(
          typename std::enable_if<
              std::is_same<int, typename result_of<F (Arguments ...) >::type>::value,
              F>::type &&func, Arguments &&...arguments)
          : file_descriptor(std::forward<F>(func)(std::forward<Arguments>
                                                      (arguments)...))
      {}

      file_descriptor(file_descriptor &&fd) noexcept
          : _fd(fd._fd)
      { fd._fd = -1; }

      file_descriptor &operator = (file_descriptor &&fd) noexcept
      {
        this->~file_descriptor();
        new (this) file_descriptor(std::move(fd));
        return *this;
      }

      file_descriptor(const file_descriptor &) = delete;

      file_descriptor &operator = (const file_descriptor &) = delete;



      ~file_descriptor();
      operator int () const noexcept { return _fd; }
      explicit operator bool () const noexcept { return _fd >= 0;}
    private:
      int _fd;
    };
  }

}
