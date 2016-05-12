/*
 * Copyright (C) 2013, 2015 LAN Xingcan
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

#ifndef LANXC_FUNCTIONAL_HPP
#define LANXC_FUNCTIONAL_HPP

#include <functional>
#include "type_traits.hpp"

/**
 * @file functional.hpp
 * @brief Functors
 * @author LAN Xingcan
 *
 * This file provides provides alternative functors implementations against
 * standard one, which adds noexcept deduction, constexpr specifier, as well
 * as C++14 Transparent Operator Functor (N3421) feature added; and some
 * interesting stuff for functional programming will also be introduced here.
 *
 * @defgroup functional Functors improved
 * @{
 */

namespace lanxc
{

  template<typename T = void>
    struct less : public std::binary_function<T, T, bool>
    {
      constexpr bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs < rhs))
      { return lhs < rhs; }
    };

  template<>
    struct less<void>
    {
      template<typename LeftType, typename RightType>
      constexpr bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs < rhs))
      { return lhs < rhs; }
    };

  template<typename T = void>
    struct greater : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs > rhs))
      { return lhs > rhs; }
    };

  template<>
    struct greater<void>
    {
      template<typename LeftType, typename RightType>
      constexpr bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs > rhs))
      { return lhs > rhs; }
    };

  template<typename T = void>
    struct less_equal : public std::binary_function<T, T, bool>
    {
      constexpr bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs <= rhs))
      { return lhs <= rhs; }
    };

  template<>
    struct less_equal<void>
    {
      template<typename LeftType, typename RightType>
      constexpr bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs <= rhs))
      { return lhs <= rhs; }
    };

  template<typename T = void>
    struct greater_equal : public std::binary_function<T, T, bool>
    {
      constexpr bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs >= rhs))
      { return lhs >= rhs; }
    };

  template<>
    struct greater_equal<void>
    {
      template<typename LeftType, typename RightType>
      constexpr bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs >= rhs))
      { return lhs >= rhs; }
    };

  template<typename T = void>
    struct equals_to : public std::binary_function<T, T, bool>
    {
      constexpr bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs == rhs))
      { return lhs == rhs; }
    };

  template<>
    struct equals_to<void>
    {
      template<typename LeftType, typename RightType>
      constexpr bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs == rhs))
      { return lhs == rhs; }
    };

  template<typename T = void>
    struct not_equals_to : public std::binary_function<T, T, bool>
    {
      constexpr bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs != rhs))
      { return lhs != rhs; }
    };

  template<>
    struct not_equals_to<void>
    {
      template<typename LeftType, typename RightType>
      constexpr bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs != rhs))
      { return lhs != rhs; }
    };

  template<typename T = void>
    struct plus : public std::binary_function<T, T, T>
    {
      constexpr T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs + rhs))
      { return lhs + rhs; }
    };

  template<>
    struct plus<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs + rhs))
        -> decltype(lhs + rhs)
      { return lhs + rhs; }
    };


  template<typename T = void>
    struct minus : public std::binary_function<T, T, T>
    {
      constexpr T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs - rhs))
      { return lhs - rhs; }
    };

  template<>
    struct minus<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs - rhs))
        -> decltype(lhs - rhs)
      { return lhs - rhs; }
    };

  template<typename T = void>
    struct multiplies : public std::binary_function<T, T, T>
    {
      constexpr T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs * rhs))
      { return lhs * rhs; }
    };

  template<>
    struct multiplies<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs * rhs))
        -> decltype(lhs * rhs)
      { return lhs * rhs; }
    };

  template<typename T = void>
    struct devides : public std::binary_function<T, T, T>
    {
      constexpr T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs / rhs))
      { return lhs / rhs; }
    };

  template<>
    struct devides<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs / rhs))
        -> decltype(lhs / rhs)
      { return lhs / rhs; }
    };

  template<typename T = void>
    struct modules : public std::binary_function<T, T, T>
    {
      constexpr T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs % rhs))
      { return lhs % rhs; }
    };

  template<>
    struct modules<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs % rhs))
        -> decltype(lhs % rhs)
      { return lhs % rhs; }
    };

  template<typename T = void>
    struct negate: public std::unary_function<T, T>
    {
      constexpr T operator () (const T &x)
        const noexcept(noexcept(-x))
      { return -x; }
    };

  template<>
    struct negate<void>
    {
      template<typename T>
      constexpr auto operator () (const T &x)
        const noexcept(noexcept(-x))
        -> decltype(-x)
      { return -x; }
    };

  template<typename T = void>
    struct logical_and : public std::binary_function<T, T, bool>
    {
      constexpr T operator () (const T &lhs,
                               const T &rhs)
        const noexcept(noexcept(lhs && rhs))
      { return lhs && rhs; }
    };

  template<>
    struct logical_and<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs && rhs))
        -> decltype(lhs && rhs)
      { return lhs && rhs; }
    };

  template<typename T = void>
    struct logical_or : public std::binary_function<T, T, bool>
    {
      constexpr T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs || rhs))
      { return lhs || rhs; }
    };

  template<>
    struct logical_or<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs || rhs))
        -> decltype(lhs || rhs)
      { return lhs || rhs; }
    };

  template<typename T = void>
    struct logical_not : public std::unary_function<T, bool>
    {
      constexpr T operator () (const T &x)
        const noexcept(noexcept(!x))
      { return !x; }
    };

  template<>
    struct logical_not<void>
    {
      template<typename T>
      constexpr auto operator () (const T &x)
        const noexcept(noexcept(!x))
        -> decltype(!x)
      { return !x; }
    };

  template<typename T = void>
    struct bit_and : public std::binary_function<T, T, T>
    {
      constexpr T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs & rhs))
      { return lhs & rhs; }
    };

  template<>
    struct bit_and<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs & rhs))
        -> decltype(lhs & rhs)
      { return lhs & rhs; }
    };

  template<typename T = void>
    struct bit_or : public std::binary_function<T, T, T>
    {
      constexpr T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs | rhs))
      { return lhs | rhs; }
    };

  template<>
    struct bit_or<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs | rhs))
        -> decltype(lhs | rhs)
      { return lhs | rhs; }
    };

  template<typename T = void>
    struct bit_xor : public std::binary_function<T, T, T>
    {
      constexpr T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs ^ rhs))
      { return lhs ^ rhs; }
    };

  template<>
    struct bit_xor<void>
    {
      template<typename LeftType, typename RightType>
      constexpr auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs ^ rhs))
        -> decltype(lhs ^ rhs)
      { return lhs ^ rhs; }
    };

  template<typename Function, typename ...OmittedArgument>
  class extend_parameter;

  template<>
  class extend_parameter<void>
  {
    template<typename ...T>
    struct tuple
    {
      template<typename F>
      using type = decltype(std::declval<F>()(std::declval<T>()...));
      template<typename R>
      using append = tuple<T...,R>;
      template<typename R>
      using prepend= tuple<R, T...>;

      template<typename F, typename ...R>
      static type<F> invoke(F &&f, T &&...args, R &&...)
      noexcept(noexcept(std::forward<F>(f)(std::forward<T>(args)...)))
      {
        return std::forward<F>(f)(std::forward<T>(args)...);
      }
    };

    template<unsigned long N, typename ...Argument>
    struct helper;

    template<typename Current, typename ...Argument>
    struct helper<0, Current, Argument...>
    {
      using type = tuple<Current, Argument...>;
    };

    template<unsigned long N, typename Current, typename ...Argument>
    struct helper<N, Current, Argument...> : helper<N - 1, Argument...>
    { };

    template<typename , typename ...>
    struct test;

    template<typename Tuple>
    struct test<Tuple>
    {
      static constexpr bool value = true;
    };
    template<typename T, typename ...A, typename R, typename ...B>
    struct test<tuple<T, A...>, R, B...>
    {
      static constexpr bool value = (sizeof...(A)) == (sizeof...(B))
        ? (std::is_convertible<T, R>::value) && test<tuple<A...>, B...>::value
        : false;
    };


    template<unsigned long N, typename T, typename S, typename ...R>
    struct invoke_helper //assume T is tuple<...>
        : invoke_helper<N - 1, typename T::template append<S>, R...>
    { };

    template<typename T, typename S, typename ...R>
    struct invoke_helper<0UL, T, S, R...> : T //assume T is tuple<...>
    {
    };

  };

  /**
   * Extends parameter for functor
   *
   * For example:
   * @code
   * auto x = [](int x) { return x + 1; }
   * lanxc::extends_parameters<decltype(x), bool, int &> f(x);
   * int v = 0;
   * f(42, true, v); // Returns 43; where `true' and `v' is ignored
   * f(42, true, 0); // Compile error: 0 is not convertible to int &
   * @endcode
   */
  template<typename Function, typename ...OmittedArgument>
  class extend_parameter : extend_parameter<void>
  {

    Function m_function;

    template<typename ...Arguments>
    using tuple_type = typename helper
        <
            sizeof...(Arguments) - sizeof...(OmittedArgument),
            Arguments...
        >::type;



    template<typename ...Arguments>
    using omit_argument_sfinae
      = typename std::enable_if
        <
          (sizeof ...(Arguments) >= sizeof ...(OmittedArgument))
              && test<tuple_type<Arguments...>, OmittedArgument...>::value,
          typename invoke_helper
            <
              sizeof...(Arguments) - sizeof...(OmittedArgument),
              tuple<>,
              Arguments &&...
            >::template type<Function>
        >::type;

    template<typename ...Arguments>
    using invoker
      = invoke_helper<
        sizeof...(Arguments) - sizeof...(OmittedArgument),
        tuple<>,
        Arguments &&...>;

    /**
     * Helper function to determine if it's noexcept when invoke the
     * underlying function with such arguments.
     */
    template<typename ...Arguments>
    static constexpr bool is_invoke_noexcept() noexcept
    {
      return noexcept(invoker<Arguments...>::invoke(std::declval<Function&>(),
          std::declval<Arguments&&>()...));
    }

  public:

    extend_parameter(Function f)
        : m_function(std::move(f))
    { }

    extend_parameter(extend_parameter &&other)
        : m_function(std::move(other.m_function))
    { }

    extend_parameter(const extend_parameter &other) = delete;


    extend_parameter &operator = (extend_parameter &&other)
    {
      this->~extend_parameter();
      new (this) extend_parameter(std::move(other));
      return *this;
    }

    extend_parameter &operator = (const extend_parameter &other) = delete;

    template<typename ...Arguments>
    omit_argument_sfinae<Arguments...> operator () (Arguments &&... args)
      noexcept(is_invoke_noexcept<Arguments...>())
    {
      return invoker<Arguments...>::invoke(m_function,
          std::forward<Arguments>(args)...);
    }
  };

  /**
   * Extends parameter for a function pointer
   *
   * For example:
   * @code
   * int add_one(int x) { return x + 1; }
   * lanxc::extends_parameters<int (*)(int), bool, int &> f(&add_one);
   * int v = 0;
   * f(42, true, v); // Returns 43; where `true' and `v' is ignored
   * f(42, true, 0); // Compile error: 0 is not convertible to int &
   * @endcode
   */
  template<typename Result, typename ...Arguments, typename ...Omitted>
  class extend_parameter<Result (*)(Arguments...), Omitted...>
  {
    Result (*m_function)(Arguments...);
  public:
    extend_parameter(Result (*func)(Arguments...)) noexcept
        : m_function(func)
    { }

    Result operator ()(Arguments &&...args, Omitted ...) const
    noexcept(noexcept(m_function(std::forward<Arguments>(args)...)))
    { return (*m_function)(std::forward<Arguments>(args)...); }
  };

  /**
   * Extends parameter for a class member function pointer
   *
   * For example:
   * @code
   * struct A
   * {
   *   int add_one(int x) { return x + 1; }
   * };
   * lanxc::extends_parameters<int (A::*)(int), bool, int &> f(&add_one);
   * int v = 0;
   * A a;
   * f(a, 42, true, v); // Returns 43; where `true' and `v' is ignored
   * f(a, 42, true, 0); // Compile error: 0 is not convertible to int &
   * @endcode
   */
  template<typename Class, typename Result,
      typename ...Arguments, typename ...Omitted>
  class extend_parameter<Result (Class::*)(Arguments...), Omitted...>
  {
    Result (Class::*m_function)(Arguments...);
  public:
    extend_parameter(Result(Class::*func)(Arguments...)) noexcept
        : m_function(func)
    { }

    Result operator () (Class &target, Arguments &&...args, Omitted &&...)
    { return (target.*m_function)(std::forward<Arguments>(args)...); }

    Result operator () (Class &&target, Arguments &&...args, Omitted &&...remains)
    { return operator() (target, std::forward<Arguments>(args)..., std::forward<Omitted>(remains)...); }
  };

  /**
   * Extends parameter for a const class member function pointer
   *
   * For example:
   * @code
   * struct A
   * {
   *   int add_one(int x) const { return x + 1; }
   * };
   * lanxc::extends_parameters<int (A::*)(int), bool, int &> f(&add_one);
   * int v = 0;
   * f(A(), 42, true, v); // Returns 43; where `true' and `v' is ignored
   * f(A(), 42, true, 0); // Compile error: 0 is not convertible to int &
   * @endcode
   */
  template<typename Class, typename Result,
      typename ...Arguments, typename ...Omitted>
  class extend_parameter<Result (Class::*)(Arguments...) const, Omitted...>
  {
    Result (Class::*m_function)(Arguments...);
  public:
    extend_parameter(Result(Class::*func)(Arguments...)) noexcept
        : m_function(func)
    { }

    Result operator () (const Class &target, Arguments &&...args, Omitted &&...)
    { return (target.*m_function)(std::forward<Arguments>(args)...); }
  };
}

/** @} */

#endif
