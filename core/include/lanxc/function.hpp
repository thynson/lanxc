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


#ifndef LANXC_FUNCTION_HPP_INCLUDED
#define LANXC_FUNCTION_HPP_INCLUDED

#include <tuple>
#include <utility>
#include <type_traits>
#include <functional>
#include <exception>
#include <typeinfo>
#include <memory>

/**
 * @file function.hpp
 * @brief Function Object
 * @author LAN Xingcan
 *
 * This file implements a morden function object against standard
 * std::function
 */

namespace lanxc
{


  template<typename>
  class function;

  struct bad_function_call : std::exception
  {
    const char *what() const noexcept override
    {
      return "Bad function call";
    }
  };

  /** @brief implementation detail */
  template<>
  class function<void>
  {
  private:

    template<typename>
    friend class function;

    /**
     * @brief function object padding
     * The size of function object is equals to size of 4 pointers,
     * The first memeber of function will be a pointer point to
     * a function which forwards arguments to the real functor; the remains
     * space is equals to size of 3 pointers
     */
    using functor_padding = void *[3];

    template<typename T>
    struct is_inplace_allocated
    {
      static const bool value
          = sizeof(T) <= sizeof(functor_padding)
            && (std::alignment_of<void *>::value
                % std::alignment_of<T>::value == 0)
            && std::is_nothrow_move_constructible<T>::value;
    };

    template<typename T>
    using inplace_allocated_sfinae =
      typename std::enable_if<is_inplace_allocated<T>::value>::type;


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
      move,
      destroy,
      typeinfo,
      target,
      const_target,
    };

    struct manager
    {
      using implement_type = std::pair<void*, const void*>
        (*)(manager *, const manager *, void *, command);

      const implement_type m_implement;
      manager(implement_type implement) noexcept
          : m_implement(implement)
      { }
    };

    template<typename Result, typename ... Arguments>
    static Result invalid_function(manager *, Arguments &&...)
    { throw bad_function_call(); }

    template<typename Function>
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

    template<typename T, typename Allocator,
        typename = typename std::conditional<
            is_inplace_allocated<T>::value, std::true_type, std::false_type>::type>
    struct manager_implement;

    template<typename Function, typename Allocator>
    struct manager_implement<Function, Allocator,
        typename std::enable_if<
            is_inplace_allocated<Function>::value, std::true_type>::type> : manager
    {
      Function m_function;
      static const std::type_info &typeinfo;

      static std::pair<void *, const void *>
      implement(manager *mgr, const manager *cmgr,  void *args, command cmd) noexcept
      {
        auto self = static_cast<manager_implement *>(mgr);
        switch (cmd)
        {
        case command::move:
          new(args) manager_implement(std::move(*self));
          return std::pair<void *, const void*>(nullptr, nullptr);
        case command::destroy:
          self->~manager_implement();
          return std::pair<void *, const void*>(nullptr, nullptr);;
        case command::typeinfo:
          return std::pair<void *, const void*>(nullptr, &typeinfo);
        case command::target:
          return std::pair<void *, const void*>(&self->m_function, nullptr);
        case command::const_target:
          return std::pair<void *, const void*>(nullptr,
              &static_cast<const manager_implement *>(cmgr)->m_function);
        }
      }

      template<typename Result, typename ...Arguments>
      static Result call(manager *mgr, Arguments &&...args)
      {
        auto self = static_cast<manager_implement *>(mgr);
        return self->m_function(std::forward<Arguments>(args)...);
      }

      manager_implement(Function &functor, const Allocator &) noexcept
        : manager(implement)
        , m_function(std::move(functor))
      { }

      manager_implement(manager_implement &&other) noexcept
        : manager(other.m_implement)
        , m_function(std::move(other.m_function))
      { }

      ~manager_implement() = default;
    };


    template<typename Function, typename Allocator>
    struct manager_implement<Function, Allocator,
        typename std::enable_if<
            !is_inplace_allocated<Function>::value, std::false_type>::type> : manager
    {
      using allocator_type = typename std::allocator_traits<Allocator>
        ::template rebind_alloc<Function>;
      using allocator_traits = std::allocator_traits<allocator_type>;

      allocator_type m_allocator;
      Function *m_function;

      static const std::type_info &typeinfo;

      template<typename Result, typename ...Arguments>
      static Result call(manager *mgr, Arguments &&...args)
      {
        auto self = static_cast<manager_implement *>(mgr);
        return (*self->m_function)(std::forward<Arguments>(args)...);
      }


      static std::pair<void*, const void *>
      implement(manager *mgr, const manager *cmgr, void *args, command cmd)
      noexcept
      {
        auto self = static_cast<manager_implement *>(mgr);

        switch (cmd)
        {
        case command::move:
          new(args) manager_implement(std::move(*self));
          return std::pair<void *, const void *>(nullptr, nullptr);
        case command::destroy:
          self->~manager_implement();
          return std::pair<void *, const void *>(nullptr, nullptr);
        case command::typeinfo:
          return std::pair<void *, const void*>(nullptr, &typeinfo);
        case command::target:
          return std::pair<void *, const void*>(&self->m_function, nullptr);
        case command::const_target:
          return std::pair<void *, const void*>(nullptr,
              &static_cast<const manager_implement *>(cmgr)->m_function);
        }
      }

      manager_implement(Function &function, const Allocator &alloc) noexcept
        : manager(implement)
        , m_allocator(alloc)
        , m_function(allocator_traits::allocate(m_allocator, 1))
      {
        allocator_traits::construct(m_allocator, m_function,
            std::move(function));
      }

      manager_implement(manager_implement &&other) noexcept
        : manager(implement)
        , m_allocator(std::move(other.m_allocator))
        , m_function(other.m_function)
      {
        other.m_function = nullptr;
      }

      ~manager_implement() noexcept
      {
        allocator_traits::destroy(m_allocator, m_function);
        allocator_traits::deallocate(m_allocator, m_function, 1);
      }
    };

  public:

    template<typename Result, typename...>
    struct member_type_definition
    {
      using result_type =  Result;
    };
    template<typename Result, typename Argument>
    struct member_type_definition<Result, Argument>
    {
      using result_type = Result;
      using argument_type = Argument;
    };
    template<typename Result, typename FirstArgument, typename SecondArgument>
    struct member_type_definition<Result, FirstArgument, SecondArgument>
    {
      using result_type = Result;
      using first_argument_type = FirstArgument;
      using second_argument_type = SecondArgument;
    };

  };

  template<typename Function, typename Allocator>
  const std::type_info &function<void>::manager_implement<
    Function, Allocator, typename std::enable_if<
        function<void>::is_inplace_allocated<Function>::value,
            std::true_type>::type>::typeinfo = typeid(Function);

  template<typename Function, typename Allocator>
  const std::type_info &function<void>::manager_implement<
    Function, Allocator, typename std::enable_if<
        !function<void>::is_inplace_allocated<Function>::value,
            std::false_type>::type>::typeinfo = typeid(Function);

  /**
   * @brief An implementation of function object alternative (but not
   *        compatitable) to std::function in spirit of morden C++
   * @author LAN Xingcan
   *
   * Since @p std::function was born in pre-C++11 era, where concept of
   * move-sematics construction was not yet presented, it requires functor
   * satisify copy-construtible rather than move-construtible. While in C++14,
   * lambda capture initialization expression was introduced, a lambda
   * captured an move-constructible but not copy-constructible object can not
   *  be accepted by @p std::function.
   * @code
   * MoveConstrutible obj; // but not copy-constructible
   * std::function<void()> = [obj=std::move(obj)] () { obj(); };   // error
   * lanxc::function<void()> = [obj=std::move(obj)] () { obj(); }; // fine
   * @endcode
   * This implementation avoid the std::function design defect, which requires
   * functor be move-construtible instead.
   *
   * @ingroup functor
   */
  template<typename Result, typename... Arguments>
  class function<Result(Arguments...)>
      : public function<void>::member_type_definition<Result, Arguments...>
  {
    using detail = function<void>;

    template<typename Function>
    using valid_functor_sfinae = typename std::enable_if<
        std::is_move_constructible<Function>::value
        && std::is_convertible<
            decltype(detail::make_functor(std::declval<Function>())
                (std::declval<Arguments>()...)),
            Result>::value>::type;

    using caller_type = Result (*)(detail::manager *, Arguments &&...);

    constexpr static caller_type noop_function
      = detail::template invalid_function<Result, Arguments...>;

    template<typename Allocator, typename Function>
    static caller_type get_caller(const Function &f) noexcept
    {
      using implement = detail::manager_implement<Function, Allocator>;
      if (detail::is_null(f))
        return noop_function;
      return implement::template call<Result, Arguments...>;
    }

  public:

    /** @brief Length of parameter list */
    static constexpr std::size_t parameter_length = sizeof ... (Arguments);

    /** @brief Type of N-th parameter */
    template<std::size_t N>
    using parameter = typename std::tuple_element<N, std::tuple<Arguments...>>::type;

    /**
     * @brief Default constructor
     * Construct an uninitialized function object
     */
    function() noexcept
        : function(nullptr)
    { }

    /**
     * @brief Constructor for null pointer
     * Construct an uninitialized function object
     */
    function(std::nullptr_t) noexcept
        : m_caller(noop_function)
    { }

    /**
     * @brief Constructor for null pointer with custom allocator
     * Construct an uninitialized function object
     */
    template<typename Allocator>
    function(std::allocator_arg_t, const Allocator &, std::nullptr_t) noexcept
        : function(nullptr)
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
    { }

    /**
     * @brief Construct a function from a functor object or (member) function
     * pointer with custom allocator
     */
    template<typename Function, typename Allocator,
        typename = valid_functor_sfinae<Function>>
    function(std::allocator_arg_t, const Allocator &a, Function f)
        noexcept(detail::is_inplace_allocated<Function>::value)
      : m_caller(get_caller<Allocator, Function>(f))
    {
      if (m_caller != noop_function)
      {
        new (m_store) detail::manager_implement<Function, Allocator>(
            detail::make_functor(f), a);
      }
    }

    /**
     * @breif Move constructor
     */
    function(function &&other) noexcept
        : function(nullptr)
    { swap(other); }

    function(const function &) = delete;

    /**
     * @breif Destructor
     */
    ~function()
    {
      if (m_caller == noop_function)
        return;
      auto impl = cast()->m_implement;
      impl(cast(), nullptr, nullptr, detail::command::destroy);
    }

    /**
     * @brief Move assignment
     */
    function &operator = (function &&other) noexcept
    {
      swap(other);
      return *this;
    }

    function &operator =(const function &other) = delete;

    /**
     * @breif Test if this function is initialized
     */
    explicit operator bool() const noexcept
    { return m_caller != noop_function; }

    /**
     * @brief Invoke this function
     */
    Result operator () (Arguments &&...args)
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
      if (lhs->m_caller != noop_function)
      {
        if (lhs->m_caller != noop_function)
        {
          std::swap(lhs->m_caller, rhs->m_caller);
          detail::functor_padding tmp;
          auto imp = rhs->cast()->m_implement;
          imp(rhs->cast(), nullptr, &tmp, detail::command::move);
          lhs->cast()->m_implement(rhs->cast(), nullptr, &rhs->m_store,
              detail::command::move);
          imp(reinterpret_cast<detail::manager *>(&tmp), nullptr, this->m_store,
              detail::command::move);
          return;
        }
        else
          std::swap(lhs, rhs);
      }

      if (rhs->m_caller != noop_function)
      {
        std::swap(lhs->m_caller, rhs->m_caller);
        auto rhs_implement = rhs->cast()->m_implement;
        rhs_implement(rhs->cast(), nullptr, &lhs->m_store,
            detail::command::move);
      }
    }

    /**
     * @breif Reconstruct this function from functor object or (member)
     * function pointer with specified allocator
     */
    template<typename Function, typename Allocator=std::allocator<void>>
    void assign(Function f, const Allocator &a = Allocator())
      noexcept(detail::is_inplace_allocated<Function>::value)
    {
      function(std::allocator_arg, a, std::move(f)).swap(*this);
    }

    /**
     * @brief Get type info of underlying functor or function pointer
     */
    const std::type_info &target_type() const noexcept
    {
      if (m_caller == noop_function)
        return typeid(nullptr);
      auto ret = (*cast()->m_implement)(nullptr, nullptr, nullptr,
          detail::command::typeinfo).second;
      return *static_cast<const std::type_info *>(ret);
    }

    /**
     * @brief Get pointer to underlying functor or function pointer
     */
    template<typename Target>
    const Target *target() const noexcept
    {
      if (m_caller == noop_function)
        return nullptr;
      if (typeid(Target) == target_type())
        return nullptr;
      auto ret = (*cast()->m_implement)(nullptr, nullptr, nullptr,
          detail::command::const_target).second;
      return static_cast<const Target *>(ret);
    }

    /**
     * @brief Get pointer to underlying functor or function pointer
     */
    template<typename Target>
    Target *target() noexcept
    {
      if (m_caller == noop_function)
        return nullptr;
      if (typeid(Target) == target_type())
        return nullptr;
      auto ret = (*cast()->m_implement)(nullptr, nullptr, nullptr,
          detail::command::const_target).first;
      return static_cast<Target *>(ret);
    }

  private:
    detail::manager *cast() noexcept
    { return reinterpret_cast<detail::manager *>(&m_store); }

    const detail::manager *cast() const noexcept
    { return reinterpret_cast<const detail::manager *>(&m_store); }

    caller_type m_caller;
    detail::functor_padding m_store;
  };

}

namespace std
{
  template<typename Result, typename... Arguments, typename Allocator>
  struct uses_allocator<lanxc::function<Result (Arguments...)>, Allocator>
    : std::true_type
  { };
}
#endif
