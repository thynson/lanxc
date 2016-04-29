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

#ifndef LANXC_TYPE_TRAITS_HPP_INCLUDED
#define LANXC_TYPE_TRAITS_HPP_INCLUDED

#include <type_traits>
#include <utility>

namespace lanxc
{

  template<typename>
  struct result_of;

  template<>
  struct result_of<void>
  {

    template<class F, class... Args>
    static inline auto invoke(F &&f, Args &&... args) ->
    decltype(std::forward<F>(f)(std::forward<Args>(args)...))
    {
      return std::forward<F>(f)(std::forward<Args>(args)...);
    }

    template<class Base, class T, class Derived>
    static inline auto invoke(T Base::*pmd, Derived &&ref) ->
    decltype(std::forward<Derived>(ref).*pmd)
    {
      return std::forward<Derived>(ref).*pmd;
    }

    template<class PMD, class Pointer>
    static inline auto invoke(PMD &&pmd, Pointer &&ptr) ->
    decltype((*std::forward<Pointer>(ptr)).*std::forward<PMD>(pmd))
    {
      return (*std::forward<Pointer>(ptr)).*std::forward<PMD>(pmd);
    }

    template<class Base, class T, class Derived, class... Args>
    static inline auto invoke(T Base::*pmf, Derived &&ref, Args &&... args) ->
    decltype((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...))
    {
      return (std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...);
    }

    template<class PMF, class Pointer, class... Args>
    static inline auto invoke(PMF &&pmf, Pointer &&ptr, Args &&... args) ->
    decltype(((*std::forward<Pointer>(ptr)).*std::forward<PMF>(pmf))(std::forward<Args>(args)...))
    {
      return ((*std::forward<Pointer>(ptr)).*std::forward<PMF>(pmf))(std::forward<Args>(args)...);
    }

    template<typename, typename = void>
    struct detail
    { };

    template<typename F, typename...Args>
    struct detail<F(Args...), decltype(void(invoke(std::declval<F>(), std::declval<Args>()...)))>
    {
      using type = decltype(invoke(std::declval<F>(), std::declval<Args>()...));
    };

  };

  template<typename T>
  struct result_of : result_of<void>::detail<T>
  { };

  template<typename T, typename ...Arguments>
  struct is_constructible_with
  {
  private:
    template<typename> struct helper {};

    template<typename U>
    static constexpr bool sfinae(helper<decltype(U(std::declval<Arguments>()...))> *)
    { return true; }

    template<typename U>
    static constexpr bool sfinae(...)
    { return false; }


  public:
    static constexpr bool value = sfinae<T>(nullptr);
  };
}
#endif
