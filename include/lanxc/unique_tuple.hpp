/*
 * Copyright (C) 2015 LAN Xingcan
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

#ifndef LANXC_UNIQUE_TUPLE_HPP_INCLUDED
#define LANXC_UNIQUE_TUPLE_HPP_INCLUDED

#include <utility>
#include <type_traits>

namespace lanxc
{

  template<typename ...T>
  struct unique_tuple;

  template<typename T>
  struct unique_tuple<T>
  {
    unique_tuple(T value)
    noexcept(std::is_nothrow_move_constructible<T>::value)
        : m_value(value)
    { }

    unique_tuple(unique_tuple &&other)
    noexcept(std::is_nothrow_move_constructible<T>::value)
        : m_value(std::move(other.m_value))
    { }

    unique_tuple(const unique_tuple &other)
    noexcept(std::is_nothrow_copy_constructible<T>::value)
        : m_value(std::move(other.m_value))
    { }

    unique_tuple &operator = (unique_tuple &&other)
    noexcept(std::is_nothrow_move_assignable<T>::value)
    {
      m_value = std::move(other.m_value);
      return *this;
    }

    unique_tuple & operator = (const unique_tuple &other)
    noexcept(std::is_nothrow_move_assignable<T>::value)
    {
      m_value = std::move(other.m_value);
      return *this;
    }

    static T &get(unique_tuple<T> &x) noexcept
    { return x.m_value;}

  private:
    T m_value;
  };


  template<>
  struct unique_tuple<>
  {
    template<typename T>
    static T &get(unique_tuple<T> &u) noexcept
    { return unique_tuple<T>::get(u); }
  };

  template<typename ...T>
  struct unique_tuple : public unique_tuple<T>...
  {

  private:
    template<typename T>
    constexpr bool nothrow_helper(T)
    { return std::is_nothrow_constructible<T>::value;}

    template<typename T, typename ...R>
    constexpr bool helper(T t, R...r)
    { return nothrow_helper(t) && nothrow_helper(r...); }


  public:
    unique_tuple(T ...value)
    noexcept(nothrow_helper(value...))
        : unique_tuple<T>(value)...
    { }

  };
}
#endif