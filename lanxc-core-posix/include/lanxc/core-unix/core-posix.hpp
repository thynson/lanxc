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
namespace lanxc
{
  namespace posix
  {
    [[noreturn]] void throw_system_error();
    [[noreturn]] void throw_system_error(int e);

    class file_descriptor
    {
    public:
      file_descriptor(int fd = -1);
      template<typename F, typename ...Arguments>
      file_descriptor(
          typename std::enable_if<
              std::is_same<int, typename result_of<F (Arguments ...) >::type>::value,
              F>::type &&func, Arguments &&...arguments)
          : file_descriptor(std::forward<F>(func)(std::forward<Arguments>
                                                      (arguments)...))
      {}
      ~file_descriptor();
      operator int () const noexcept;
      operator bool () const noexcept;
    private:
      int _fd;
    };
  }

}
