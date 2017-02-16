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
  private:
    struct implement
    {
      template<class F, class... A>
      static inline auto
      invoke(F &&f, A &&... args) ->
      decltype(std::forward<F>(f)(std::forward<A>(args)...));

      template<class B, class T, class D>
      static inline auto
      invoke(T B::*pmd, D &&ref) ->
      decltype(std::forward<D>(ref).*pmd);

      template<class PMD, class P>
      static inline auto invoke(PMD &&pmd, P &&ptr) ->
      decltype((*std::forward<P>(ptr)).*std::forward<PMD>(pmd));

      template<class B, class T, class D, class... Args>
      static inline auto invoke(T B::*pmf, D &&ref, Args &&... args) ->
      decltype((std::forward<D>(ref).*pmf)(std::forward<Args>(args)...));

      template<class PMF, class P, class... A>
      static inline auto invoke(PMF &&pmf, P &&ptr, A &&... args) ->
      decltype(((*std::forward<P>(ptr)).*std::forward<PMF>(pmf))
          (std::forward<A>(args)...));

      template<typename, typename = void>
      struct detail
      { };

      template<typename F, typename...Args>
      struct detail
        <
          F(Args...),
          decltype(void(invoke(std::declval<F>(), std::declval<Args>()...)))
        >
      {
        using type = decltype(invoke(std::declval<F>(), std::declval<Args>()...));
      };
    };
  public:
    template<typename T>
    using sfinae = implement::detail<T>;
  };

  template<typename T>
  struct result_of : result_of<void>::sfinae<T>
  { };

  template<typename T, typename ...Arguments>
  struct is_constructible_with
  {
  private:
    template<typename> struct helper {};

    template<typename U>
    static constexpr bool
    sfinae(helper<decltype(U(std::declval<Arguments>()...))> *)
    { return true; }

    template<typename U>
    static constexpr bool sfinae(...)
    { return false; }


  public:
    static constexpr bool value = sfinae<T>(nullptr);
  };
}
#endif
