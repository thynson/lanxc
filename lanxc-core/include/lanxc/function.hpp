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

#pragma once

#include "type_traits.hpp"

#include <lanxc/config.hpp>

#include <utility>
#include <algorithm>
#include <type_traits>
#include <functional>
#include <exception>
#include <memory>

/**
 * @file function.hpp
 * @brief Function Object
 * @author LAN Xingcan
 *
 * This file implements a modern function object against standard
 * std::function
 */

namespace lanxc
{


  template<typename>
  class function;

  struct bad_function_call final: std::exception
  {
    const char *what() const noexcept override
    {
      return "Bad function call";
    }
  };

  /** @brief implementation detail */
  template<>
  class LANXC_CORE_EXPORT function<void> final
  {
  private:

    template<typename>
    friend class function;

    /*
     * The size of function object is equals to size of 4 pointers. The first
     * member will be a pointer point to a function which forwards arguments
     * to the real functor; the second member is a pointer point to implement
     * details. And the remains space is equals to size of 2 pointers.
     */

    using functor_padding = void *[3];

    template<typename T>
    struct is_inplace_allocated
    {
      static const bool value
          = sizeof(T) <= (sizeof(void*) * 2)
            && (std::alignment_of<void *>::value
                % std::alignment_of<T>::value == 0)
            && std::is_nothrow_move_constructible<T>::value;
    };

    template<bool V, typename T>
    using inplace_allocated_sfinae =
      typename std::enable_if<is_inplace_allocated<T>::value == V>::type;


    template<typename T>
    static bool is_null(const T &) noexcept
    { return false; }

    template<typename Result, typename... Arguments>
    static bool
    is_null(Result (*const &fp)(Arguments...)) noexcept
    { return fp == nullptr; }

    template<typename Result, typename Class, typename... Arguments>
    static bool
    is_null(Result (Class::* const &fp)(Arguments...)) noexcept
    { return fp == nullptr; }

    template<typename Result, typename Class, typename... Arguments>
    static bool
    is_null(Result (Class::* const &fp)(Arguments...) const) noexcept
    { return fp == nullptr; }

    /**
     * @brief Functor implementation commands
     * These commands is defined to avoid implementing @ref manager with
     * virtual functions.
     */
    enum class command
    {
      construct,
      destroy,
    };

    struct abstract
    {
      using implement_type =
        void (*)(abstract *, void *, command);

      const implement_type m_implement;
      explicit abstract(implement_type implement) noexcept
          : m_implement(implement)
      { }
    };

    template<typename Result, typename ... Arguments>
    static Result invalid_function(abstract *, Arguments ...)
    { throw bad_function_call(); }

    template<typename Function, typename = typename
      std::enable_if<std::is_move_constructible<Function>::value>::type>
    static Function &make_functor(Function &&f)
    { return f; }

    template<typename Result, typename Class, typename... Arguments>
    static auto make_functor(Result (Class::*func)(Arguments &&...))
      -> decltype(std::mem_fn(func))
    { return std::mem_fn(func); }


    template<typename Result, typename Class, typename... Arguments>
    static auto make_functor(Result (Class::*func)(Arguments &&...) const)
      -> decltype(std::mem_fn(func))
    { return std::mem_fn(func); }

    template<typename T, typename A, typename = void>
    struct concrete;

    template<typename Function, typename Allocator>
    struct concrete
      < Function,
        Allocator,
        inplace_allocated_sfinae<true, Function>
      > final : abstract
    {
      Function m_function;

      static void
      implement(abstract *mgr, void *args, command cmd) noexcept
      {
        auto self = static_cast<concrete *>(mgr);
        switch (cmd)
        {
        case command::construct:
          new(args) concrete(std::move(*self));
          break;
        case command::destroy:
          self->~concrete();
          break;
        }
      }

      template<typename Result, typename ...Arguments>
      static Result call(abstract *mgr, Arguments ...args)
      {
        auto self = static_cast<concrete *>(mgr);
        return self->m_function(std::forward<Arguments>(args)...);
      }

      concrete(Function &functor, const Allocator &) noexcept
        : abstract(implement)
        , m_function(std::move(functor))
      { }

      concrete(concrete &&other) noexcept
        : abstract(other.m_implement)
        , m_function(std::move(other.m_function))
      { }

      concrete &operator = (concrete &&) = delete;

      ~concrete() = default;

      concrete(const concrete &) = delete;
      concrete &operator = (const concrete &) = delete;
    };


    template<typename Function, typename Allocator>
    struct concrete
      < Function,
        Allocator,
        inplace_allocated_sfinae<false, Function>
      > final : abstract
    {
      using allocator_type = typename std::allocator_traits<Allocator>
        ::template rebind_alloc<Function>;
      using allocator_traits = std::allocator_traits<allocator_type>;

      allocator_type m_allocator;
      Function *m_function;

      template<typename Result, typename ...Arguments>
      static Result call(abstract *mgr, Arguments ...args)
      {
        auto self = static_cast<concrete *>(mgr);
        return (*self->m_function)(std::forward<Arguments>(args)...);
      }

      static void
      implement(abstract *mgr, void *args, command cmd)
      noexcept
      {
        auto self = static_cast<concrete *>(mgr);

        switch (cmd)
        {
        case command::construct:
          new(args) concrete(std::move(*self));
          break;
        case command::destroy:
          self->~concrete();
          break;
        }
      }

      concrete(Function &function, const Allocator &alloc) noexcept
        : abstract(implement)
        , m_allocator(alloc)
        , m_function(allocator_traits::allocate(m_allocator, 1))
      {
        allocator_traits::construct(m_allocator, m_function,
            std::move(function));
      }

      concrete(concrete &&other) noexcept
        : abstract(implement)
        , m_allocator(std::move(other.m_allocator))
        , m_function(other.m_function)
      {
        other.m_function = nullptr;
      }

      ~concrete() noexcept
      {
        allocator_traits::destroy(m_allocator, m_function);
        allocator_traits::deallocate(m_allocator, m_function, 1);
      }

      concrete &operator = (concrete &&) = delete;
      concrete(const concrete &) = delete;
      concrete &operator = (const concrete &) = delete;
    };

  };

  /**
   * @brief An implementation of function object alternative (but not
   *        compatible) to std::function in spirit of modern C++
   * @author LAN Xingcan
   *
   * Since @p std::function was born in pre-C++11 era, while concept of
   * move-semantic construction was not yet presented, it requires functor
   * satisfy copy-constructible rather than move-constructible. While in C++14,
   * lambda capture initialization expression was introduced, a lambda
   * captured an move-constructible but not copy-constructible object can not
   * be accepted by @a std::function.
   * @code
   * MoveConstructible obj; // but not copy-constructible
   * std::function<void()> = [obj=std::move(obj)] () { obj(); };   // error
   * lanxc::function<void()> = [obj=std::move(obj)] () { obj(); }; // fine
   * @endcode
   * This implementation avoid the @a std::function design defect, which
   * requires functor be move-constructible instead.
   *
   * @ingroup functor
   */
  template<typename Result, typename... Arguments>
  class LANXC_CORE_EXPORT function<Result(Arguments...)> final
  {
    using detail = function<void>;

    template<typename Function>
    using valid_functor_sfinae = typename std::enable_if<
        std::is_convertible<typename result_of<Function(Arguments...)>::type, Result>::value
        && ! std::is_same<Function, function>::value
    >::type;

    using caller_type = Result (*)(detail::abstract *, Arguments ...);

    static constexpr caller_type noop_function
       = function<void>::template invalid_function<Result, Arguments...>;

    template<typename Allocator, typename Function>
    static caller_type get_caller(const Function &f) noexcept
    {
      using implement = detail::concrete<Function, Allocator>;
      if (detail::is_null(f))
        return noop_function;
      return implement::template call<Result, Arguments...>;
    }

  public:

    using result = Result;
    using arguments = std::tuple<Arguments...>;

    /**
     * @brief Default constructor
     * Construct an uninitialized function object
     */
    function() noexcept
        : m_caller(noop_function)
    { }

    /**
     * @brief Constructor for null pointer
     * Construct an uninitialized function object
     */
    function(std::nullptr_t) noexcept
        : function()
    { }

    /**
     * @brief Move constructor
     */
    function(function &&other) noexcept
        : function()
    { swap(other); }

    function(const function &) = delete;

    /**
     * @brief Constructor for null pointer with custom allocator
     * Construct an uninitialized function object
     */
    template<typename Allocator>
    function(std::allocator_arg_t, const Allocator &, std::nullptr_t) noexcept
        : function()
    { }

    /**
     * @brief Construct a function from a functor object or (member) function
     * pointer
     */
    template<typename Function, typename = valid_functor_sfinae<Function>>
    function(Function functor)
        noexcept(detail::is_inplace_allocated<Function>::value)
        : function(std::allocator_arg, std::allocator<void>(),
                   std::move(functor))
    {
      static_assert(std::is_move_constructible<Function>::value,
                    "functor object must be move constructible");
    }

    /**
     * @brief Construct a function from a functor object or (member) function
     * pointer with custom allocator
     */
    template<typename Function,
             typename Allocator,
             typename = valid_functor_sfinae<Function>>
    function(std::allocator_arg_t, const Allocator &a, Function f)
        noexcept(detail::is_inplace_allocated<Function>::value)
      : m_caller(get_caller<Allocator, Function>(f))
    {
      static_assert(std::is_move_constructible<Function>::value,
                    "functor object must be move constructible");
      if (m_caller != noop_function)
      {
        new (m_store) detail::concrete<Function, Allocator>(
            detail::make_functor(f), a);
      }
    }

    /**
     * @brief Destructor
     */
    ~function()
    {
      if (m_caller == noop_function)
        return;
      auto impl = cast()->m_implement;
      impl(cast(), nullptr, detail::command::destroy);
    }

    /**
     * @brief Move assignment
     */
    function &operator = (function &&other) noexcept
    {
      swap(other);
      return *this;
    }

    function &operator =(const function &) = delete;

    /**
     * @brief Test if this function is initialized
     */
    explicit operator bool() const noexcept
    { return m_caller != noop_function; }

    /**
     * @brief Invoke this function
     */
    Result operator () (Arguments ...args)
    {
      return (*m_caller)(cast(), std::forward<Arguments>(args)...);
    }

    /**
     * @brief Swap two function
     */
    void swap(function &other) noexcept
    {
      function *lhs = this, *rhs = &other;
      if (lhs == rhs)
        return;
      if (*lhs)
      {
        if (*rhs)
        {
          std::swap(lhs->m_caller, rhs->m_caller);
          detail::functor_padding tmp;
          auto imp = rhs->cast()->m_implement;
          imp(rhs->cast(), &tmp, detail::command::construct);
          lhs->cast()->m_implement(lhs->cast(), &rhs->m_store,
              detail::command::construct);
          imp(reinterpret_cast<detail::abstract *>(&tmp),
              &lhs->m_store,
              detail::command::construct);
          return;
        }
        else
          std::swap<function*>(lhs, rhs);
      }

      if (*rhs)
      {
        std::swap(lhs->m_caller, rhs->m_caller);
        auto rhs_implement = rhs->cast()->m_implement;
        rhs_implement(rhs->cast(), &lhs->m_store,
            detail::command::construct);
      }
    }

    /**
     * @brief Reconstruct this function from functor object or (member)
     * function pointer with specified allocator
     */
    template<typename Function, typename Allocator=std::allocator<void>>
    void assign(Function f, const Allocator &a = Allocator())
      noexcept(detail::is_inplace_allocated<Function>::value)
    {
      function(std::allocator_arg, a, std::move(f)).swap(*this);
    }

  private:

    detail::abstract *cast() noexcept
    { return reinterpret_cast<detail::abstract *>(&m_store); }

    const detail::abstract *cast() const noexcept
    { return reinterpret_cast<const detail::abstract *>(&m_store); }

    caller_type m_caller;
    detail::functor_padding m_store;
  };

  extern template class LANXC_CORE_EXPORT function<void()>;
  extern template class LANXC_CORE_EXPORT function<bool()>;

}

namespace std
{
  template<typename Result, typename... Arguments, typename Allocator>
  struct uses_allocator<lanxc::function<Result (Arguments...)>, Allocator>
    : std::true_type
  { };
}

