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
 * standard one, which is lack of noexcept deduction, as well as C++14
 * Transparent Operator Functor (N3421) feature added; and some interesting
 * stuff for functional programming will also be introduced here.
 *
 * @defgroup functional Functors improved
 * @{
 */

namespace lanxc
{

  template<typename T = void>
    struct less : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs < rhs))
      { return lhs < rhs; }
    };

  template<>
    struct less<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
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
    struct greater<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs > rhs))
      { return lhs > rhs; }
    };

  template<typename T = void>
    struct less_equal : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs <= rhs))
      { return lhs <= rhs; }
    };

  template<>
    struct less_equal<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs <= rhs))
      { return lhs <= rhs; }
    };

  template<typename T = void>
    struct greater_equal : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs >= rhs))
      { return lhs >= rhs; }
    };

  template<>
    struct greater_equal<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs >= rhs))
      { return lhs >= rhs; }
    };

  template<typename T = void>
    struct equals_to : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs == rhs))
      { return lhs == rhs; }
    };

  template<>
    struct equals_to<void>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs == rhs))
      { return lhs == rhs; }
    };

  template<typename T = void>
    struct not_equals_to : public std::binary_function<T, T, bool>
    {
      bool operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs != rhs))
      { return lhs != rhs; }
    };

  template<>
    struct not_equals_to<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      bool operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs != rhs))
      { return lhs != rhs; }
    };

  template<typename T = void>
    struct plus : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs + rhs))
      { return lhs + rhs; }
    };

  template<>
    struct plus<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs + rhs))
        -> decltype(lhs + rhs)
      { return lhs + rhs; }
    };


  template<typename T = void>
    struct minus : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs - rhs))
      { return lhs - rhs; }
    };

  template<>
    struct minus<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs - rhs))
        -> decltype(lhs - rhs)
      { return lhs - rhs; }
    };

  template<typename T = void>
    struct multiplies : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs * rhs))
      { return lhs * rhs; }
    };

  template<>
    struct multiplies<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs * rhs))
        -> decltype(lhs * rhs)
      { return lhs * rhs; }
    };

  template<typename T = void>
    struct devides : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs / rhs))
      { return lhs / rhs; }
    };

  template<>
    struct devides<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs / rhs))
        -> decltype(lhs / rhs)
      { return lhs / rhs; }
    };

  template<typename T = void>
    struct modules : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs % rhs))
      { return lhs % rhs; }
    };

  template<>
    struct modules<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs % rhs))
        -> decltype(lhs % rhs)
      { return lhs % rhs; }
    };

  template<typename T = void>
    struct negate: public std::unary_function<T, T>
    {
      T operator () (const T &x)
        const noexcept(noexcept(-x))
      { return -x; }
    };

  template<>
    struct negate<void> : public std::unary_function<void, void>
    {
      template<typename T>
      auto operator () (const T &x)
        const noexcept(noexcept(-x))
        -> decltype(-x)
      { return -x; }
    };

  template<typename T = void>
    struct logical_and : public std::binary_function<T, T, bool>
    {
      T operator () (const T &lhs,
                               const T &rhs)
        const noexcept(noexcept(lhs && rhs))
      { return lhs && rhs; }
    };

  template<>
    struct logical_and<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs && rhs))
        -> decltype(lhs && rhs)
      { return lhs && rhs; }
    };

  template<typename T = void>
    struct logical_or : public std::binary_function<T, T, bool>
    {
      T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs || rhs))
      { return lhs || rhs; }
    };

  template<>
    struct logical_or<void> : public std::binary_function<void, void, bool>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs || rhs))
        -> decltype(lhs || rhs)
      { return lhs || rhs; }
    };

  template<typename T = void>
    struct logical_not : public std::unary_function<T, bool>
    {
      T operator () (const T &x)
        const noexcept(noexcept(!x))
      { return !x; }
    };

  template<>
    struct logical_not<void> : public std::unary_function<void, bool>
    {
      template<typename T>
      auto operator () (const T &x)
        const noexcept(noexcept(!x))
        -> decltype(!x)
      { return !x; }
    };

  template<typename T = void>
    struct bit_and : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs & rhs))
      { return lhs & rhs; }
    };

  template<>
    struct bit_and<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs & rhs))
        -> decltype(lhs & rhs)
      { return lhs & rhs; }
    };

  template<typename T = void>
    struct bit_or : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs | rhs))
      { return lhs | rhs; }
    };

  template<>
    struct bit_or<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs | rhs))
        -> decltype(lhs | rhs)
      { return lhs | rhs; }
    };

  template<typename T = void>
    struct bit_xor : public std::binary_function<T, T, T>
    {
      T operator () (const T &lhs, const T &rhs)
        const noexcept(noexcept(lhs ^ rhs))
      { return lhs ^ rhs; }
    };

  template<>
    struct bit_xor<void> : public std::binary_function<void, void, void>
    {
      template<typename LeftType, typename RightType>
      auto operator () (const LeftType &lhs, const RightType &rhs)
        const noexcept(noexcept(lhs ^ rhs))
        -> decltype(lhs ^ rhs)
      { return lhs ^ rhs; }
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
  class extend_parameter
  {
    /** @brief Convert variadic template pack to tuple */
    template<unsigned long N, typename ...Argument>
    struct helper;

    template<typename Current, typename ...Argument>
    struct helper<0, Current, Argument...>
    {
      using type = std::tuple<Current, Argument...>;
    };

    template<unsigned long N, typename Current, typename ...Argument>
    struct helper<N, Current, Argument...> : helper<N - 1, Argument...>
    { };

    /** @brief Test if types in @p TupleA is convertible to @p TupleB */
    template<typename TupleA, typename TupleB>
    struct test;

    template<typename A, typename B>
    struct test<std::tuple<A>, std::tuple<B>>
    {
      static constexpr bool value = std::is_convertible<A, B>::value;
    };

    template<typename A, typename ...T, typename B, typename ...R>
    struct test<std::tuple<A, T...>, std::tuple<B, R...>>
    {
      static constexpr bool value
        = (sizeof ... (T) == sizeof ...(R))
          ? (std::is_convertible<A, B>::value
             && (sizeof ...(T) == 0
                 || test<std::tuple<T...>, std::tuple<R...>>::value))
          : false;
    };

    template<unsigned long N, typename F, typename T, typename ...R>
    struct invoke_helper
    { };

    template<typename F, typename ...A, typename T, typename ...R>
    struct invoke_helper<0UL, F, std::tuple<A...>, T, R...>
    {
      using type = decltype(std::declval<F>()(std::declval<A>()...));

      static type invoke(F &f, A &&...args, T &&, R &&...)
      noexcept(noexcept(f(std::forward<A>(args)...)))
      {
        return f(std::forward<A>(args)...);
      }
    };

    template<unsigned long N, typename F, typename ...A, typename T,
        typename ...R>
    struct invoke_helper<N, F, std::tuple<A...>, T, R...>
        : invoke_helper<N - 1, F, std::tuple<A..., T>, R...>
    { };

    Function m_function;

    template<typename ...Arguments>
    using omit_argument_sfinae
      = typename std::enable_if<
          sizeof ...(Arguments) >= sizeof ... (OmittedArgument)
            && test<std::tuple<OmittedArgument...>,
              typename helper<sizeof...(Arguments) - sizeof...(OmittedArgument),
                  Arguments...>::type>::value,
          typename invoke_helper<
              sizeof...(Arguments) - sizeof...(OmittedArgument),
              Function,
              std::tuple<>,
              Arguments &&...>::type>::type;

    template<typename ...Arguments>
    using invoker
      = invoke_helper<
        sizeof...(Arguments) - sizeof...(OmittedArgument),
        Function,
        std::tuple<>,
        Arguments &&...>;

    /**
     * Helper function to determin if it's noexcept when invoke the
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
