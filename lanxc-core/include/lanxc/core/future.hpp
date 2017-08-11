/*
 * Copyright (C) 2017 LAN Xingcan
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

#include <lanxc/core/task_context.hpp>
#include <lanxc/function.hpp>
#include <lanxc/config.hpp>

#include <exception>

namespace lanxc
{

  template<typename ...Value>
  class promise;

  template<typename ...Value>
  class future;

  /**
   * @brief Indicate that the future has been cancelled without a reason
   *
   * If a @a promise is destructed when neither promise<T...>::set_result,
   * promise<T...>::set_exception nor promise<T...>::set_exception_ptr has
   * been called. The corresponding future will catch an exception of this
   * class.
   */
  class LANXC_CORE_EXPORT promise_cancelled : public std::exception
  {
  public:
    const char *what() const noexcept override
    {
      return "promise cancelled";
    }
  };

  /**
   * @brief Indicate that an operation is not permitted for a future
   *
   * If a @a future has been committed or chained by another future, any
   * subsequential operation will result in this an exception of this type
   * being thrown.
   */
  class LANXC_CORE_EXPORT invalid_future : public std::exception
  {
  public:
    const char *what() const noexcept override
    {
      return "this future is invalid, possibly because .then_apply(), "
          ".caught() or .commit() was already called for this future.";
    }
  };
  /**
   * @brief The promise for a future
   *
   * A promise guards a process that a future is getting delivered.
   *
   * The destruction behavior of a promise depends on its state, which
   * initially set to failure with reason of cancelled. Call to #fulfill
   * of an promise changes its state to success and the arguments of this
   * call will be staged as result. While call to #reject_by_exception_ptr
   * or #reject changes its state to failure and the argument of such call
   * will be staged as the reason of failure. When the promise is being
   * destructed, the staged result or reason of failure will be delivered
   * to the corresponding future.
   */
  template<typename ...Value>
  class LANXC_CORE_EXPORT promise final
  {
    template<typename ...> friend class future;
  public:
    ~promise() noexcept
    {
      if (_detail)
      {
        _detail->deliver();
        _detail = nullptr;
      }
    };

    promise(promise &&p) noexcept
      : _detail { nullptr }
    { std::swap(_detail, p._detail); }

    promise &operator = (promise &&p) noexcept
    {
      std::swap(_detail, p._detail);
      return *this;
    }

    promise(const promise &) = delete;
    promise &operator = (const promise &) = delete;

    /**
     * @brief Stage given values to fulfill the promise
     * @param values Values to be staged
     * @note The value is not delivered until this promise is destructed
     */
    void fulfill(Value ...values)
    {
      _detail->set_result(std::move(values)...);
    }

    /**
     * @brief Stage given exception pointer to reject the promise
     * @param e The exception pointer
     * @note The exception pointer is not delivered until this promise is
     * destructed
     */
    void reject_by_exception_ptr(std::exception_ptr e)
    {
      _detail->set_exception_ptr(std::move(e));
    }

    /**
     * @brief Stage given exception instance to reject the promise
     * @tparam E Type of exception
     * @param e Instance of exception
     * @note The exception is not delivered until this promise is destructed
     */
    template<typename E>
    void reject(E e)
    {
      reject_by_exception_ptr(std::make_exception_ptr(e));
    }
  private:

    using defer_task_type
        = function<std::shared_ptr<deferred>(task_context &, promise)>;
    using fulfill_type = function<void(Value...)>;
    using reject_type = function<void(std::exception_ptr)>;

    struct detail
    {
      defer_task_type _routine;
      fulfill_type _fulfill;
      reject_type  _reject;
      function<void()> _delivery;
      std::shared_ptr<deferred> _next;
      task_context *_task_context;
      std::exception_ptr _exception_ptr = nullptr;

      detail(defer_task_type routine) noexcept
          : _routine(std::move(routine))
          , _fulfill{[](Value...) {}}
          , _reject{[](std::exception_ptr) {  }}
          , _delivery
              {
                  [this]
                  { _reject(std::make_exception_ptr(promise_cancelled())); }
              }
          , _task_context{nullptr}
      { }

      ~detail() = default;

      void set_result(Value ...result)
      {
        _delivery = std::bind([this](Value ...value)
                            { _fulfill(std::move(value)...);},
                            std::move(result)...);
        _exception_ptr = nullptr;
      }

      void set_exception_ptr(std::exception_ptr e)
      {
        _exception_ptr = e;
        _delivery = [this] { _reject(std::move(_exception_ptr)); };
      }

      void set_fulfill_action(fulfill_type f) noexcept
      {
        _fulfill.swap(f);
      }

      void set_reject_action(reject_type f) noexcept
      {
        _reject.swap(f);
      }

      static std::shared_ptr<deferred>
      start(task_context &ctx, std::shared_ptr<detail> d)
      {
        d->_task_context = &ctx;
        auto &r = d->_routine;
        return r(ctx, promise(std::move(d)));

      }

      void deliver()
      {
        _next = _task_context->defer(std::move(_delivery));
      }
    };

    promise(std::shared_ptr<detail> x = nullptr) noexcept
        : _detail(std::move(x))
    { }

    std::shared_ptr<detail> _detail;

  };


  extern template class LANXC_CORE_EXPORT promise<>;

  /**
   * @brief Represents values that may be available in the future
   * @tparam Value Types of values
   */
  template<typename ...Value>
  class LANXC_CORE_EXPORT future final
  {
    template<typename ...>
    friend class lanxc::future;

    using promise_type = promise<Value...>;
    using detail_type  = typename promise<Value...>::detail;
    using detail_ptr   = std::shared_ptr<detail_type>;



    template<typename ...V>
    using promise_type_for = promise<V...>;

    template<typename ...V>
    using detail_type_for  = typename promise_type_for<V...>::detail;

    template<typename ...V>
    using detail_ptr_for   = std::shared_ptr<detail_type_for<V...>>;

    template<typename V>
    using then_value_routine = function<V(Value...)>;

    template<typename ...V>
    using then_future_routine = function<future<V...>(Value...)>;

    using then_void_routine = function<void(Value...)>;

    template<typename E, typename V>
    using caught_value_routine = function<V(E&)>;

    template<typename E, typename ...V>
    using caught_future_routine = function<future<V...>(E&)>;

    template<typename E>
    using caught_void_routine = function<void(E&)>;


    struct base
    {
      detail_ptr _detail;

      base(detail_ptr p) noexcept
          : _detail { std::move(p) }
      { }
    };

    struct initiator : base
    {
      function<void(promise_type)> _routine;

      initiator(detail_ptr d, function<void(promise_type)> r) noexcept;

      void operator () ();
    };

    struct dispatcher
    {
      function<void(promise_type)> _routine;
      dispatcher(function<void(promise_type)> r) noexcept;

      std::shared_ptr<deferred>
      operator () (task_context &ctx, promise_type p);
    };

    struct reject_functor : base
    {
      using base::base;
      void operator () (std::exception_ptr e);
    };

    struct forward_functor : base
    {
      using base::base;
      void operator () (Value ...);
    };

    template<typename ...R>
    struct cancel_functor : base
    {
      using base::base;
      void operator () (Value ...);
    };


    template<typename ...V>
    struct then_future_functor
    {
      detail_ptr_for<V...> _detail;
      then_future_routine<V...> _routine;
      std::shared_ptr<deferred> _task;
      then_future_functor(detail_ptr_for<V...> d,
                          then_future_routine<V...> r) noexcept;

      void operator () (Value ...value);
    };

    template<typename ...V>
    struct then_future_detail : base
    {
      then_future_routine<V...> _routine;

      then_future_detail(detail_ptr current,
                         then_future_routine<V...> routine) noexcept;

      std::shared_ptr<deferred> start(task_context &ctx, promise<V...> next);
    };

    template<typename ...V>
    struct then_future_action
    {
      std::shared_ptr<then_future_detail<V...>> _detail;

      std::shared_ptr<deferred>
      operator () (task_context &ctx, promise<V...> p);

      then_future_action(detail_ptr current,
                         then_future_routine<V...> routine);
    };

    template<typename V>
    struct then_value_functor
    {
      detail_ptr_for<V> _next;
      then_value_routine<V> _routine;
      std::shared_ptr<deferred> _task;
      then_value_functor(detail_ptr_for<V> d,
                         then_value_routine<V> r) noexcept;
      void operator () (Value ...value);
    };


    template<typename V>
    struct then_value_detail : public base
    {
      then_value_routine<V> _routine;

      then_value_detail(detail_ptr current,
                        then_value_routine<V> routine) noexcept;

      std::shared_ptr<deferred> start(task_context &ctx, promise<V> next);
    };

    template<typename V>
    struct then_value_action
    {
      std::shared_ptr<then_value_detail<V>> _detail;

      std::shared_ptr<deferred>
      operator () (task_context &ctx, promise<V> p);

      then_value_action(detail_ptr current, then_value_routine<V> routine);
    };

    struct then_void_functor
    {
      detail_ptr_for<> _detail;
      then_void_routine _routine;
      std::shared_ptr<deferred> _task;
      then_void_functor(detail_ptr_for<> d, then_void_routine r) noexcept;
      void operator () (Value ...value);
    };

    struct then_void_detail : base
    {
      then_void_routine _routine;
      then_void_detail(detail_ptr current, then_void_routine routine) noexcept;
      std::shared_ptr<deferred> start(task_context &ctx, promise<> next);
    };
    struct then_void_action
    {
      std::shared_ptr<then_void_detail> _detail;

      std::shared_ptr<deferred>
      operator () (task_context &ctx, promise<> p);

      then_void_action(detail_ptr current, then_void_routine routine);
    };

    template<typename E, typename ...V>
    struct caught_future_functor
    {
      detail_ptr_for<V...> _detail;
      caught_future_routine<E, V...> _routine;
      std::shared_ptr<deferred> _task;

      caught_future_functor(detail_ptr_for<V...> d,
                          caught_future_routine<E, V...> r) noexcept;

      void operator () (std::exception_ptr);
    };

    template<typename E, typename ...V>
    struct caught_future_detail : base
    {
      caught_future_routine<E, V...> _routine;

      caught_future_detail(detail_ptr current,
                           caught_future_routine<E, V...> routine) noexcept;

      std::shared_ptr<deferred> start(task_context &ctx, promise<V...> next);
    };

    template<typename E, typename ...V>
    struct caught_future_action
    {
      std::shared_ptr<caught_future_detail<E, V...>> _detail;

      std::shared_ptr<deferred>
      operator () (task_context &ctx, promise<V...> p);

      caught_future_action(detail_ptr current,
                           caught_future_routine<E, V...> routine);
    };

    template<typename E, typename V>
    struct caught_value_functor
    {
      detail_ptr_for<V> _detail;
      caught_value_routine<E, V> _routine;
      std::shared_ptr<deferred> _task;
      caught_value_functor(detail_ptr_for<V> d,
                           caught_value_routine<E, V> r) noexcept;
      void operator () (std::exception_ptr);
    };


    template<typename E, typename V>
    struct caught_value_detail : public base
    {
      caught_value_routine<E, V> _routine;

      caught_value_detail(detail_ptr current,
                          caught_value_routine<E, V> routine) noexcept;

      std::shared_ptr<deferred> start(task_context &ctx, promise<V> next);
    };

    template<typename E, typename V>
    struct caught_value_action
    {
      std::shared_ptr<caught_value_detail<E, V>> _detail;

      std::shared_ptr<deferred>
      operator () (task_context &ctx, promise<V> p);

      caught_value_action(detail_ptr current,
                          caught_value_routine<E, V> routine);
    };


    template<typename E>
    struct caught_void_functor
    {
      detail_ptr_for<> _detail;
      caught_void_routine<E> _routine;
      std::shared_ptr<deferred> _task;
      caught_void_functor(detail_ptr_for<> d,
                          caught_void_routine<E> r) noexcept;
      void operator () (std::exception_ptr);
    };

    template<typename E>
    struct caught_void_detail : base
    {
      caught_void_routine<E> _routine;
      caught_void_detail(detail_ptr current,
                         caught_void_routine<E> routine) noexcept;
      std::shared_ptr<deferred> start(task_context &ctx, promise<> next);
    };

    template<typename E>
    struct caught_void_action
    {
      std::shared_ptr<caught_void_detail<E>> _detail;

      std::shared_ptr<deferred>
      operator () (task_context &ctx, promise<> p);

      caught_void_action(detail_ptr current, caught_void_routine<E> routine);

    };

    template<typename F, typename V = typename result_of<F(Value...)>::type>
    struct then_type
    {
      using future_type = future<V>;
      static future<V> construct(future f, then_value_routine<V> r);
    };

    template<typename F, typename ...V>
    struct then_type<F, future<V...>>
    {
      using future_type = future<V...>;
      static future<V...> construct(future f, then_future_routine<V...> r);
    };

    template<typename F>
    struct then_type<F, void>
    {
      using future_type = future<>;
      static future<> construct(future f, then_void_routine r);
    };

    template<typename E, typename F,
             typename V = typename result_of<F(E&)>::type>
    struct caught_type
    {
      using future_type = future<V>;

      static future<V>
      construct(future f, caught_value_routine<E, V> r);
    };

    template<typename E, typename F, typename ...V>
    struct caught_type<E, F, future<V...>>
    {
      using future_type = future<V...>;

      static future<V...>
      construct(future f, caught_future_routine<E, V...> r);
    };

    template<typename E, typename F>
    struct caught_type<E, F, void>
    {
      using future_type = future<>;

      static future<>
      construct(future f, caught_void_routine<E> r);
    };

  public:

    /**
     * @brief Create a future that will be rejected with specified exception
     * @tparam E Type of exception
     * @param e Instance info exception
     * @return A future
     */
    template<typename E>
    static future reject(E &e)
    {
      auto exp = std::make_exception_ptr(e);

      return future<Value...>
          {
              [exp](promise_type p) { p.reject_by_exception_ptr(exp); }
          };

    }

    /**
     * @brief Create a future that will be resolved to specified values
     * @param values Values to be resolved
     * @return A future
     */
    static future resolve(Value ...values)
    {
      return future<Value...>
          {
            std::bind([](promise_type p, Value &...values)
                  {
                    p.fulfill(std::move(values)...);
                  }, std::placeholders::_1, std::move(values)...)
          };

    }

    /**
     * @brief Construct a future with a functor
     * @param r the functor that will fulfill the promise
     */

    future(function<void(promise<Value...>)> r)
        : _detail_ptr
          { std::make_shared<detail_type>(dispatcher(std::move(r))) }
    { }


    /**
     * @brief Setup function to call after fulfilling of this future
     * @tparam R The type of function to call
     * @param r The instance of function
     * @return A new @a future
     *
     * The actual type of the returned future is determined by the return
     * type of @a F.
     *   - If @a F returns a @ref future, this function returns a future of
     *     same type too
     *   - If @a F returns @c void, this function returns @ref future<>
     *   - If @a F returns other type of @c T, this function returns
     *     @ref future<T>
     */
    template<typename R>
    typename then_type<R>::future_type
    then(R &&r)
    {
      return then_type<R>::construct(std::move(*this), std::move(r));
    }

    /**
     * @brief Setup an error handler for this future
     * @tparam E The type of exception to catch
     * @tparam R The type of error handler
     * @param f The instance of error handler function
     * @return A new @a future
     *
     * The actual type of the returned future is determined by the return
     * type of @a F.
     *   - If @a F returns a @ref future, this function returns a future of
     *     same type too
     *   - If @a F returns @c void, this function returns @ref future<>
     *   - If @a F returns other type of @c T, this function returns
     *     @ref future<T>
     */
    template<typename E, typename R>
    typename caught_type<E, R>::future_type
    caught(R &&f)
    {
      return caught_type<E, R>::construct(std::move(*this), std::move(f));
    };

    /**
     * @brief Resolve this future within an executor
     * @param ctx The executor
     */
    std::shared_ptr<deferred> start(task_context &ctx)
    {
      return detail_type::start(ctx, std::move(_detail_ptr));
    }

  private:

    /**
     * @brief Internal constructor
     * @param p A shared pointer to promise implement detail
     */
    future(detail_ptr p) noexcept
        : _detail_ptr{ std::move(p) }
    { }

    detail_ptr _detail_ptr;
  };

  extern template class LANXC_CORE_EXPORT future<>;


  template<typename ...Value>
  template<typename F, typename V>
  future<V> future<Value...>::then_type<F, V>::
  construct(future f, then_value_routine<V> r)
  {
    return future_type
        {
            std::make_shared<detail_type_for<V>>(
                then_value_action<V>(std::move(f._detail_ptr), std::move(r)))
        };
  }

  template<typename ...Value>
  template<typename F>
  future<> future<Value...>::then_type<F, void>::
  construct(future f, then_void_routine r)
  {
    return future_type
        {
            std::make_shared<detail_type_for<>>(
                then_void_action(std::move(f._detail_ptr), std::move(r)))
        };
  }

  template<typename ...Value>
  template<typename F, typename ...V>
  future<V...> future<Value...>::then_type<F, future<V...>>::
  construct(future f, then_future_routine<V...> r)
  {
    return future_type
        {
            std::make_shared<detail_type_for<V...>>(
                then_future_action<V...>(std::move(f._detail_ptr),
                std::move(r)))
        };
  }

  template<typename ...Value>
  template<typename E, typename F, typename V>
  future<V> future<Value...>::caught_type<E, F, V>::
  construct(future f, caught_value_routine<E, V> r)
  {
    return future_type
        {
            std::make_shared<detail_type_for<V>>(
                caught_value_action<E,V> (std::move(f._detail_ptr),
                    std::move(r)))
        };
  }

  template<typename ...Value>
  template<typename E, typename F>
  future<> future<Value...>::caught_type<E, F, void>::
  construct(future f, caught_void_routine<E> r)
  {
    return future_type
        {
            std::make_shared<detail_type_for<>>(
                caught_void_action<E> (std::move(f._detail_ptr),
                                       std::move(r)))
        };
  }

  template<typename ...Value>
  template<typename E, typename F, typename ...V>
  future<V...> future<Value...>::caught_type<E, F, future<V...>>::
  construct(future f, caught_future_routine<E, V...> r)
  {
    return future_type
        {
            std::make_shared<detail_type_for<V...>>(
                caught_future_action<E, V...>(std::move(f._detail_ptr),
                    std::move(r)))
        };
  }

  template<typename ...Value>
  void future<Value...>::reject_functor::
  operator () (std::exception_ptr e)
  {
    promise_type p { this->_detail };
    p.reject_by_exception_ptr(e);
  }

  template<typename ...Value>
  template<typename ...R>
  void future<Value...>::cancel_functor<R...>::
  operator () (Value ...)
  {
    typename future<R...>::promise_type p {this->_detail};
    p.reject(promise_cancelled());
  }


  template<typename ...Value>
  void future<Value...>::forward_functor::operator () (Value ...v)
  {
    promise<Value...> p{this->_detail};
    try
    {
      p.fulfill(std::move(v)...);
    }
    catch (...)
    {
      p.reject_by_exception_ptr(std::current_exception());
    }
  }


  /**
   * @brief Functor that initiate the progress of resolving future values
   */
  template<typename ...Value>
  future<Value...>::initiator::
  initiator(detail_ptr d, function<void(promise_type)> r) noexcept
      : base { std::move(d) }
      , _routine { std::move(r) }
  { }

  template<typename ...Value>
  void future<Value...>::initiator:: operator () ()
  { return _routine(promise_type(this->_detail)); }

  /**
   * @brief Functor that dispatch the initiator
   */
  template<typename ...Value>
  future<Value...>::dispatcher::
  dispatcher(function<void(promise_type)> r) noexcept
      : _routine { std::move(r) }
  { }
  template<typename ...Value>
  std::shared_ptr<deferred>
  future<Value...>::dispatcher::
  operator () (task_context &ctx, promise_type p)
  {
    return ctx.defer(
        initiator{std::move(p._detail), std::move(_routine)});
  }

  template<typename ...Value>
  template<typename ...V>
  future<Value...>::then_future_functor<V...>::
  then_future_functor(detail_ptr_for<V...> d,
                      then_future_routine<V...> r) noexcept
      : _detail { std::move(d) }
      , _routine { std::move(r) }
  { }

  template<typename ...Value>
  template<typename ...V>
  void future<Value...>::then_future_functor<V...>::
  operator () (Value ...value)
  {
    try
    {
      task_context &ctx = *_detail->_task_context;
      auto f = _routine(std::move(value)...);
      f._detail_ptr->set_fulfill_action(
           typename future<V...>::forward_functor(_detail));
      f._detail_ptr->set_reject_action(
           typename future<V...>::reject_functor(_detail));
      _task = f.start(ctx);
    }
    catch (...)
    {
      promise<V...> p { _detail };
      p.reject_by_exception_ptr(std::current_exception());
    }
  }

  template<typename ...Value>
  template<typename ...V>
  future<Value...>::then_future_detail<V...>::
  then_future_detail(detail_ptr current,
                     then_future_routine<V...> routine) noexcept
      : base { std::move(current)}
      , _routine{std::move(routine)}
  { }

  template<typename ...Value>
  template<typename ...V>
  std::shared_ptr<deferred>
  future<Value...>::then_future_detail<V...>::
  start(task_context &ctx, promise<V...> next)
  {
    auto ptr = std::move(next._detail);
    this->_detail->set_fulfill_action(
        then_future_functor<V...>(ptr, std::move(_routine)));
    this->_detail->set_reject_action(
        typename future<V...>::reject_functor{std::move(ptr)});
    return detail_type::start(ctx, std::move(this->_detail));
  }


  template<typename ...Value>
  template<typename ...V>
  future<Value...>::then_future_action<V...>::
  then_future_action(detail_ptr current,
                     then_future_routine<V...> routine)

      : _detail
        {
            std::make_shared<future::then_future_detail <V...> >(
                std::move(current), std::move(routine))
        }
  { }

  template<typename ...Value>
  template<typename ...V>
  std::shared_ptr<deferred>
  future<Value...>::then_future_action<V...>::
  operator () (task_context &ctx, promise<V...> p)
  {
    return _detail->start(ctx, std::move(p));
  }

  template<typename ...Value>
  template<typename V>
  future<Value...>::then_value_functor<V>::
  then_value_functor(detail_ptr_for<V> next,
                     then_value_routine<V> routine) noexcept
      : _next{std::move(next)}
      , _routine{std::move(routine)}
  { }

  template<typename ...Value>
  template<typename V>
  void future<Value...>::then_value_functor<V>::
  operator () (Value ... value)
  {
    promise<V> p {_next};
    try
    {
      p.fulfill(_routine(std::forward<Value>(value)...));
    }
    catch (...)
    {
      p.reject_by_exception_ptr(std::current_exception());
    }
  }


  template<typename ...Value>
  template<typename V>
  future<Value...>::then_value_detail<V>::
  then_value_detail(detail_ptr current, then_value_routine<V> routine) noexcept
      : base { std::move(current) }
      , _routine { std::move(routine) }
  { }

  template<typename ...Value>
  template<typename V>
  std::shared_ptr<deferred>
  future<Value...>::then_value_detail<V>::
  start(task_context &ctx, promise<V> next)
  {
    detail_ptr_for<V> ptr = std::move(next._detail);
    this->_detail->set_fulfill_action(
        then_value_functor<V>(ptr, std::move(_routine)));
    this->_detail->set_reject_action(
        typename future<V>::reject_functor(std::move(ptr)));
    return detail_type::start(ctx, std::move(this->_detail));
  }

  template<typename ...Value>
  template<typename V>
  future<Value...>::then_value_action<V>::
  then_value_action(detail_ptr current, then_value_routine<V> routine)
      : _detail
      {
          std::make_shared<future::then_value_detail<V>>(std::move(current),
                                                         std::move(routine))
      }
  { }

  template<typename ...Value>
  template<typename V>
  std::shared_ptr<deferred>
  future<Value...>::then_value_action<V>::
  operator ()(task_context &ctx, promise<V> p)
  {
    return _detail->start(ctx, std::move(p));
  }

  template<typename ...Value>
  future<Value...>::then_void_functor::
  then_void_functor(std::shared_ptr<promise<>::detail> d,
                    then_void_routine routine) noexcept
      : _detail{std::move(d)}
      , _routine{std::move(routine)}
  { }

  template<typename ...Value>
  void future<Value...>::then_void_functor::
  operator () (Value ... value)
  {
    promise<> p {_detail};
    try
    {
      _routine(std::forward<Value>(value)...);
      p.fulfill();
    }
    catch (...)
    {
      p.reject_by_exception_ptr(std::current_exception());
    }
  }
  template<typename ...Value>
  future<Value...>::then_void_detail::
  then_void_detail(detail_ptr current, then_void_routine routine) noexcept
      : base{std::move(current)}
      , _routine{std::move(routine)}
  { }

  template<typename ...Value>
  std::shared_ptr<deferred>
  future<Value...>::then_void_detail::
  start(task_context &ctx, promise<> next)
  {
    auto ptr = std::move(next._detail);
    this->_detail->set_fulfill_action(
        then_void_functor(ptr, std::move(_routine)));
    this->_detail->set_reject_action(
        future<>::reject_functor(std::move(ptr)));
    return detail_type::start(ctx, std::move(this->_detail));
  }

  template<typename ...Value>
  future<Value...>::then_void_action::
  then_void_action(detail_ptr current, then_void_routine done)
      : _detail
      {
          std::make_shared<future::then_void_detail>(std::move(current),
                                                     std::move(done))
      }
  { }

  template<typename ...Value>
  std::shared_ptr<deferred>
  future<Value...>::then_void_action::
  operator () (task_context &ctx, promise<> p)
  { return _detail->start(ctx, std::move(p)); }

  template<typename ...Value>
  template<typename E, typename ...V>
  future<Value...>::caught_future_functor<E, V...>::
  caught_future_functor(detail_ptr_for<V...> ptr,
                        caught_future_routine<E, V...> f) noexcept
      : _detail { std::move(ptr) }
      , _routine { std::move(f) }
  { }

  template<typename ...Value>
  template<typename E, typename ...V>
  void future<Value...>::caught_future_functor<E, V...>::
  operator () (std::exception_ptr e)
  {
    try
    {
      std::rethrow_exception(std::move(e));
    }
    catch(E &e)
    {
      auto f = _routine(e);
      f._detail_ptr->set_fulfill_action(
           typename future<V...>::forward_functor(_detail));
      f._detail_ptr->set_reject_action(
           typename future<V...>::reject_functor(_detail));
      _task = f.start(*_detail->_task_context);
    }
    catch(...)
    {
      promise<V...> p { _detail };
      p.reject_by_exception_ptr(std::current_exception());
    }
  }

  template<typename ...Value>
  template<typename E, typename ...V>
  future<Value...>::caught_future_detail<E, V...>::
  caught_future_detail(detail_ptr current,
                       caught_future_routine<E, V...> routine) noexcept
      : base {std::move(current)}
      , _routine {std::move(routine)}
  { }

  template<typename ...Value>
  template<typename E, typename ...V>
  std::shared_ptr<deferred>
  future<Value...>::caught_future_detail<E, V...>::
  start(task_context &ctx, promise<V...> next)
  {
    auto ptr = std::move(next._detail);
    this->_detail->set_fulfill_action(cancel_functor<V...>(ptr));
    this->_detail->set_reject_action(
        caught_future_functor<E, V...>(std::move(ptr),
                                       std::move(_routine)));
    return detail_type::start(ctx, std::move(this->_detail));
  }

  template<typename ...Value>
  template<typename E, typename ...V>
  future<Value...>::caught_future_action<E, V...>::
  caught_future_action(detail_ptr current,
                       caught_future_routine<E, V...> routine)
      : _detail
      {
          std::make_shared<future::caught_future_detail<E,V...>>(
              std::move(current), std::move(routine))
      }
  { }

  template<typename ...Value>
  template<typename E, typename ...V>
  std::shared_ptr<deferred>
  future<Value...>::caught_future_action<E, V...>::
  operator () (task_context &ctx,
               promise<V...> p)
  {
    return _detail->start(ctx, std::move(p));
  }

  template<typename ...Value>
  template<typename E, typename V>
  future<Value...>::caught_value_functor<E, V>::
  caught_value_functor(detail_ptr_for<V> ptr,
                       caught_value_routine<E, V> f) noexcept
      : _detail { std::move(ptr) }
      , _routine { std::move(f) }
  { }

  template<typename ...Value>
  template<typename E, typename V>
  void future<Value...>::caught_value_functor<E, V>::
  operator () (std::exception_ptr e)
  {
    try
    {
      std::rethrow_exception(std::move(e));
    }
    catch(E &e)
    {
      promise<V> p { _detail };
      p.fulfill(_routine(e));
    }
    catch(...)
    {
      promise<V> p { _detail };
      p.reject_by_exception_ptr(std::current_exception());
    }
  }

  template<typename ...Value>
  template<typename E, typename V>
  future<Value...>::caught_value_detail<E, V>::
  caught_value_detail(detail_ptr current,
                      caught_value_routine<E, V> routine) noexcept
      : base {std::move(current)}
      , _routine {std::move(routine)}
  { }

  template<typename ...Value>
  template<typename E, typename V>
  std::shared_ptr<deferred>
  future<Value...>::caught_value_detail<E, V>::
  start(task_context &ctx, promise<V> next)
  {
    auto ptr = std::move(next._detail);
    this->_detail->set_fulfill_action(cancel_functor<V>(ptr));
    this->_detail->set_reject_action(
        caught_value_functor<E, V>(std::move(ptr), std::move(_routine)));
    return detail_type::start(ctx, std::move(this->_detail));
  }

  template<typename ...Value>
  template<typename E, typename V>
  future<Value...>::caught_value_action<E, V>::
  caught_value_action(detail_ptr current,
                      caught_value_routine<E, V> routine)
      : _detail
      {
          std::make_shared<future::caught_value_detail<E, V>>(
              std::move(current), std::move(routine))
      }
  { }

  template<typename ...Value>
  template<typename E, typename V>
  std::shared_ptr<deferred>
  future<Value...>::caught_value_action<E, V>::
  operator () (task_context &ctx, promise<V> p)
  {
    return _detail->start(ctx, std::move(p));
  }

  template<typename ...Value>
  template<typename E>
  future<Value...>::caught_void_functor<E>::
  caught_void_functor(detail_ptr_for<> ptr,
                      caught_void_routine<E> f) noexcept
      : _detail { std::move(ptr) }
      , _routine { std::move(f) }
  { }

  template<typename ...Value>
  template<typename E>
  void future<Value...>::caught_void_functor<E>::
  operator () (std::exception_ptr e)
  {
    try
    {
      std::rethrow_exception(std::move(e));
    }
    catch(E &e)
    {
      promise<> p { _detail };
      _routine(e);
      p.fulfill();
    }
    catch(...)
    {
      promise<> p { _detail };
      p.reject_by_exception_ptr(std::current_exception());
    }
  }

  template<typename ...Value>
  template<typename E>
  future<Value...>::caught_void_detail<E>::
  caught_void_detail(detail_ptr current,
                     caught_void_routine<E> routine) noexcept
      : base {std::move(current)}
      , _routine {std::move(routine)}
  { }

  template<typename ...Value>
  template<typename E>
  std::shared_ptr<deferred>
  future<Value...>::caught_void_detail<E>::
  start(task_context &ctx, promise<> next)
  {
    auto ptr = std::move(next._detail);
    this->_detail->set_fulfill_action(cancel_functor<>(ptr));
    this->_detail->set_reject_action(
        caught_void_functor<E>(ptr, std::move(_routine)));
    return detail_type::start(ctx, std::move(this->_detail));
  }

  template<typename ...Value>
  template<typename E>
  future<Value...>::caught_void_action<E>::
  caught_void_action(detail_ptr current, caught_void_routine<E> routine)
      : _detail
      {
          std::make_shared<future::caught_void_detail<E>>(
              std::move(current), std::move(routine))
      }
  { }

  template<typename ...Value>
  template<typename E>
  std::shared_ptr<deferred>
  future<Value...>::caught_void_action<E>::
  operator () (task_context &ctx, promise<> p)
  {
    return _detail->start(ctx, std::move(p));
  }

}
