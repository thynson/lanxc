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
      return "this future is invalid, possibly because .then_apply(), .caught()"
          " or .commit() was already called for this future.";
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
  class LANXC_CORE_EXPORT promise
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

    void fulfill(Value ...result)
    {
      _detail->set_result(std::move(result)...);
    }

    void reject_by_exception_ptr(std::exception_ptr e)
    {
      _detail->set_exception_ptr(e);
    }

    template<typename E>
    void reject(E e)
    {
      reject_by_exception_ptr(std::make_exception_ptr(e));
    }
  private:

    using routine_type
        = function<std::shared_ptr<deferred>(task_context &, promise)>;

    struct detail : public std::enable_shared_from_this<detail>
    {
      routine_type _routine;
      function<void(Value...)> _fulfill;
      function<void(std::exception_ptr)> _reject;
      function<void()> _delivery;
      std::shared_ptr<deferred> _next;
      task_context *_task_context;

      detail(routine_type routine) noexcept
          : _routine(std::move(routine))
          , _fulfill{[](Value...) {}}
          , _reject{[](std::exception_ptr){}}
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
      }

      void set_exception_ptr(std::exception_ptr e)
      {
        _delivery = [this, e] { _reject(e); };
      }

      void set_fulfill_action(function<void(Value...)> f) noexcept
      {
        _fulfill.swap(f);
      }

      void set_reject_action(function<void(std::exception_ptr)> f) noexcept
      {
        _reject.swap(f);
      }

      std::shared_ptr<deferred> start(task_context &ctx)
      {
        _task_context = &ctx;
        return _routine(ctx, promise(this->shared_from_this()));
      }

      void deliver()
      {
        _next = _task_context->defer_immediate(std::move(_delivery));
      }
    };

    std::shared_ptr<detail> _detail;

    promise(std::shared_ptr<detail> x) noexcept
        : _detail(std::move(x))
    { }

    promise(std::nullptr_t = nullptr) noexcept
        : _detail {nullptr}
    { }

  };

  template<typename ...Value>
  class LANXC_CORE_EXPORT future
  {
    template<typename ...>
    friend class future;

    using promise_type = promise<Value...>;
    using detail_type = typename promise<Value...>::detail;
    using detail_ptr = std::shared_ptr<detail_type>;

    template<typename F, typename R = typename result_of<F(Value...)>::type>
    struct then_type
    {
      using future_type = future<R>;
      static future_type
      construct_future(future self, function<R (Value...)> f)
      {
        return future_type
            {
              std::make_shared<typename future_type::detail_type>(
                  then_value_action<R>(std::move(self._detail_ptr), std::move(f)) )
            };
      }
    };
    template<typename F>
    struct then_type<F, void>
    {
      using future_type = future<>;

      static future_type
      construct_future(future self, function<void (Value...)> f)
      {
        return future_type
          {
            std::make_shared<typename future_type::detail_type>(
                then_void_action(std::move(self._detail_ptr), std::move(f)) )
          };
      }

    };

    template<typename F, typename ...V>
    struct then_type<F, future<V...>>
    {
      using future_type = future<V...>;
      static future_type
      construct_future(future self, function<future_type(Value...)> f)
      {
        return future_type
          {
            std::make_shared<typename future_type::detail_type>(
                then_future_action<V...>(std::move(self._detail_ptr), std::move(f)) )
          };
      }
    };

    template<typename E, typename F,
             typename R = typename result_of<F(E&)>::type>
    struct caught_exception_type
    {
      using future_type = future<R>;

      static future_type
      construct_future(future self, function<R(E&)> f)
      {
        return future_type
          {
            std::make_shared<typename future_type::detail_type>(
            caught_value_action<E,R> (std::move(self._detail_ptr), std::move(f)))
          };
      }
    };
    template<typename E, typename F>
    struct caught_exception_type<E, F, void>
    {
      using future_type = future<>;

      static future_type
      construct_future(future self, lanxc::function<void(E&)> f)
      {
        return future_type
          {
            std::make_shared<typename future_type::detail_type>(
              caught_void_action<E> (std::move(self._detail_ptr), std::move(f)))
          };
      }
    };

    template<typename E, typename F, typename ...V>
    struct caught_exception_type<E, F, future<V...>>
    {
      using future_type = future<V...>;

      static future_type
      construct_future(future self, function<future_type(E&)> f)
      {
        return future_type
          {
            std::make_shared<typename future_type::detail_type>(
              caught_future_action<E, V...>(std::move(self._detail_ptr),
                                            std::move(f)))
          };
      }
    };

    ;

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
     * @param values values to be resolved
     * @return A future
     */
    static future resolve(Value ...values)
    {
      return future<Value...>
          {
            std::bind([](promise_type p, Value ...values)
                  {
                    p.fulfill(std::move(values)...);
                  }, std::placeholders::_1, std::move(values)...)
          };

    }

    /**
     * @brief Construct a future with a functor
     * @param initiator the functor that will fulfill the promise
     */

    future(function<void(promise<Value...>)> f) noexcept
        : _detail_ptr
          { std::make_shared<detail_type>(initiator(std::move(f))) }
    { }


    /**
     * @brief Setup function to call after fulfilling of this future
     * @tparam F The type of function to call
     * @param f The instance of function
     * @return A new @a future
     *
     * The actual type of the returned future is determined by the return
     * type of @a F.
     *   - If @a F returns a @a future, this function returns a future of
     *     same type too
     *   - If @a F returns @a void, this function returns `future<>`
     *   - If @a F returns other type of `T`, this function returns
     *     `future<T>`
     */
    template<typename F>
    typename then_type<F>::future_type
    then(F &&f)
    {
      return then_type<F>::construct_future(std::move(*this), std::move(f));
    }

    /**
     * @brief Setup an error handler for this future
     * @tparam E The type of exception to catch
     * @tparam F The type of error handler
     * @param f The instance of error handler function
     * @return A new @a future
     *
     * The actual type of the returned future is determined by the return
     * type of @a F.
     *   - If @a F returns a @a future, this function returns a future of
     *     same type too
     *   - If @a F returns @a void, this function returns `future<>`
     *   - If @a F returns other type of `T`, this function returns
     *     `future<T>`
     */
    template<typename E, typename F>
    typename caught_exception_type<E, F>::future_type
    caught(F &&f)
    {
      return caught_exception_type<E, F>::construct_future(std::move(*this),
                                                 std::move(f));
    };

    /**
     * @brief Resolve this future within an executor
     * @param ctx The executor
     */
    std::shared_ptr<deferred> start(task_context &ctx)
    {
      return _detail_ptr->start(ctx);
    }

  private:

    future(detail_ptr p)
        : _detail_ptr{ std::move(p) }
    { }

    struct initiator
    {
      function<void(promise_type)> _routine;
      promise_type _promise;
      initiator(lanxc::function<void(promise_type)> r)
          : _routine { std::move(r) }
      { }
      std::shared_ptr<deferred>
      operator () (task_context &ctx, promise_type p)
      {
        _promise = std::move(p);
        return ctx.defer_immediate(
            [this]
            {
              _routine(std::move(_promise));
            });
      }
    };

    template<typename ...R>
    struct then_future_action
    {
      struct detail;
      std::shared_ptr<detail> _detail;
      std::shared_ptr<deferred> operator () (task_context &t, promise<R...> p);
      then_future_action(detail_ptr current,
                         function<future<R...>(Value...)> routine);
    };

    template<typename R>
    struct then_value_action
    {
      struct detail;
      std::shared_ptr<detail> _detail;
      std::shared_ptr<deferred> operator () (task_context &t, promise<R> p);
      then_value_action(detail_ptr current,
                        function<R(Value...)> routine);
    };


    struct then_void_action
    {
      struct detail;
      std::shared_ptr<detail> _detail;
      std::shared_ptr<deferred> operator () (task_context &t, promise<> p);
      then_void_action(detail_ptr current, function<void(Value...)> done);
    };

    template<typename E, typename ...R>
    struct caught_future_action
    {
      struct detail;
      std::shared_ptr<detail> _detail;
      std::shared_ptr<deferred> operator () (task_context &, promise<R...> next);
      caught_future_action(detail_ptr current,
                           function<future<R...>(E&)> routine);
    };

    template<typename E, typename R>
    struct caught_value_action
    {
      struct detail;
      std::shared_ptr<detail> _detail;
      std::shared_ptr<deferred> operator () (task_context &, promise<R> next);
      caught_value_action(detail_ptr current,
                          function<R(E&)> routine);
    };


    template<typename E>
    struct caught_void_action
    {
      struct detail;
      std::shared_ptr<detail> _detail;
      std::shared_ptr<deferred> operator () (task_context &, promise<> next);
      caught_void_action(detail_ptr current, function<void(E&)> done);
    };

    detail_ptr _detail_ptr;
  };

  template<typename ...Value>
  template<typename ...R>
  struct future<Value...>::then_future_action<R...>::detail
  {
    detail_ptr _current;
    typename future<R...>::detail_ptr _tmp;
    promise<R...> _next;
    function<future<R...>(Value...)> _routine;
    std::shared_ptr<deferred> _awaiting_task;

    detail(detail_ptr current,
           function<future<R...>(Value...)> routine)
        : _current{std::move(current)}
        , _next{}
        , _routine{std::move(routine)}
    { }

    ~detail() = default;

    struct fulfill_functor
    {
      std::shared_ptr<detail> _p;
      fulfill_functor(std::shared_ptr<detail> p)
          : _p(std::move(p))
      { }

      void operator () (R ... r)
      {
        _p->_next.fulfill(std::move(r)...);
        _p->_next = nullptr;
      }
    };


    std::shared_ptr<deferred> start(task_context &ctx, promise<R...> next)
    {
      _next = std::move(next);
      _current->set_fulfill_action(
                  [this] (Value ...value)
                  {
                    try
                    {
                      auto f = _routine(std::move(value)...);
                      _tmp = f._detail_ptr;
                      _tmp->set_fulfill_action(
                           [this](R ...r)
                           {
                             _next.fulfill(std::move(r)...);
                             _next._detail->deliver();
                           });
                      _tmp->set_reject_action(
                           [this] (std::exception_ptr e)
                           {
                             _next.reject_by_exception_ptr(e);
                             _next._detail->deliver();
                           });
                      _awaiting_task = _tmp->start(*_current->_task_context);
                    }
                    catch (...)
                    {
                      _next.reject_by_exception_ptr(std::current_exception());
                    }
                  }
              );
      _current->set_reject_action(
                  [this] (std::exception_ptr e)
                  {
                    _next.reject_by_exception_ptr(e);
                    _next._detail->deliver();
                  }
              );
      return _current->start(ctx);
    }
  };

  template<typename ...Value>
  template<typename ...R>
  std::shared_ptr<deferred>
  future<Value...>::then_future_action<R...>
  ::operator () (task_context &ctx, promise<R...> next)
  {
    return _detail->start(ctx, std::move(next));
  }

  template<typename ...Value>
  template<typename ...R>
  future<Value...>::then_future_action<R...>
  ::then_future_action(detail_ptr current,
                       function<future<R...>(Value...)> routine)
    : _detail
      { std::make_shared<detail>(std::move(current), std::move(routine)) }
  { }


  template<typename ...Value>
  template<typename R>
  struct future<Value...>::then_value_action<R>::detail
      : public std::enable_shared_from_this<detail>
  {
    detail_ptr _current;
    promise<R>   _next;
    function<R(Value...)> _routine;

    detail(detail_ptr current,
           function<R(Value...)> routine)
        : _current { std::move(current) }
        , _next {}
        , _routine { std::move(routine) } {}

    ~detail() = default;

    std::shared_ptr<deferred> start(task_context &ctx, promise<R> next)
    {
      _next = std::move(next);
      _current->set_fulfill_action(
          [this](Value ...value)
          {
            try
            {
              _next.fulfill(_routine(std::forward<Value>(value)...));
            }
            catch (...)
            {
              _next.reject_by_exception_ptr(std::current_exception());
            }
            _next._detail->deliver();
          }
      );
      _current->set_reject_action(
          [this](std::exception_ptr e)
          {
            _next.reject_by_exception_ptr(e);
            _next._detail->deliver();
          }
      );
      return _current->start(ctx);
    }
  };

  template<typename ...Value>
  template<typename R>
  std::shared_ptr<deferred>
  future<Value...>::then_value_action<R>
  ::operator () (task_context &ctx, promise<R> next)
  {
    return _detail->start(ctx, std::move(next));
  }

  template<typename ...Value>
  template<typename R>
  future<Value...>::then_value_action<R>
  ::then_value_action(detail_ptr current,
                      function<R(Value...)> routine)
    : _detail{std::make_shared<detail>(std::move(current), std::move(routine))}
  {
  }

  template<typename ...Value>
  struct future<Value...>::then_void_action::detail
  {
    detail_ptr _current;
    promise<> _next;
    function<void(Value...)> _routine;

    detail(detail_ptr current,
           function<void(Value...)> routine)
        : _current{std::move(current)}
        , _next{}
        , _routine{std::move(routine)}
    { }

    std::shared_ptr<deferred> start(task_context &ctx, promise<> next)
    {
      _next = std::move(next);
      _current->set_fulfill_action(
          [this] (Value ...value)
          {
            std::exception_ptr e;
            try
            {
              _routine(std::move(value)...);
            }
            catch (...)
            {
              e = std::current_exception();
            }
            if (e) _next.reject_by_exception_ptr(e);
            else _next.fulfill();
            _next._detail->deliver();
          }
      );
      _current->set_reject_action(
          [this] (std::exception_ptr e)
          {
            _next.reject_by_exception_ptr(e);
            _next._detail->deliver();
          }
      );
      return _current->start(ctx);
    }
  };

  template<typename ...Value>
  std::shared_ptr<deferred> future<Value...>::then_void_action
  ::operator () (task_context &ctx, promise<> next)
  {
    return _detail->start(ctx, std::move(next));
  }

  template<typename ...Value>
  future<Value...>::then_void_action
  ::then_void_action(detail_ptr current, function<void(Value...)> done)
      : _detail
      { std::make_shared<detail>(std::move(current), std::move(done)) }
  { }

  template<typename ...Value>
  template<typename E, typename ...R>
  struct future<Value...>::caught_future_action<E, R...>::detail
  {
    detail_ptr _current;
    typename future<R...>::detail_ptr _tmp;
    promise<R...> _next;
    function<future<R...>(E &)> _routine;
    std::shared_ptr<deferred> _awaiting_task;

    detail(detail_ptr current,
           function<future<R...>(E &)> routine)
        : _current{std::move(current)}
        , _next{}
        , _routine{std::move(routine)}
    { }

    std::shared_ptr<deferred> start(task_context &ctx, promise<R...> next)
    {
      _next = std::move(next);
      _current->set_fulfill_action(
          [this, &ctx] (Value ...)
          {
            _next.reject(promise_cancelled());
            _next._detail->deliver();
          }
      );
      _current->set_reject_action(
          [this] (std::exception_ptr p)
          {
            try
            {
              std::rethrow_exception(p);
            }
            catch (E &e)
            {
              auto f = _routine(e);
              _tmp = f._detail_ptr;
              f._detail_ptr->set_fulfill_action(
                   [this] (Value ...value)
                   {
                     try
                     {
                       _next.fulfill(std::move(value)...);
                     }
                     catch (...)
                     {
                       _next.reject_by_exception_ptr(std::current_exception());
                     }
                     _next._detail->deliver();
                   });
              f._detail_ptr->set_reject_action(
                   [this](std::exception_ptr e)
                   {
                     _next.reject_by_exception_ptr(e);
                     _next._detail->deliver();
                   });
              _awaiting_task = f._detail_ptr->start(*_current->_task_context);
            }
            catch (...)
            {
              _next.reject_by_exception_ptr(std::current_exception());
              _next._detail->deliver();
            }
          }
      );
      return _current->start(ctx);

    }
  };

  template<typename ...Value>
  template<typename E, typename ...R>
  future<Value...>::caught_future_action<E, R...>
  ::caught_future_action(detail_ptr current,
                         function<future<R...>(E&)> routine)
      : _detail
      { std::make_shared<detail>(std::move(current), std::move(routine)) }
  { }

  template<typename ...Value>
  template<typename E, typename ...R>
  std::shared_ptr<deferred> future<Value...>::caught_future_action<E, R...>
  ::operator ()(task_context &ctx, promise<R...> p)
  {
    return _detail->start(ctx, std::move(p));
  }

  template<typename ...Value>
  template<typename E, typename R>
  struct future<Value...>::caught_value_action<E, R>::detail
  {
    detail_ptr _current;
    promise<R> _next;
    function<R(E &)> _routine;
    std::shared_ptr<deferred> _awaited_task;

    detail(detail_ptr current,
           function<R(E &)> routine)
        : _current{std::move(current)}
        , _next{}
        , _routine{std::move(routine)}
    { }

    std::shared_ptr<deferred> start(task_context &ctx, promise<R> next)
    {
      _next = std::move(next);
      _current->set_fulfill_action(
          [this] (Value ...)
          {
            _next.reject(promise_cancelled());
            _next._detail->deliver();
          }
      );
      _current->set_reject_action(
          [this] (std::exception_ptr p)
          {
            try
            {
              std::rethrow_exception(p);
            }
            catch(E &e)
            {
              _next.fulfill(_routine(e));
            }
            catch(...)
            {
              _next.reject_by_exception_ptr(std::current_exception());
            }

            _next._detail->deliver();
          }
      );
      return _current->start(ctx);
    }
  };

  template<typename ...Value>
  template<typename E, typename R>
  future<Value...>::caught_value_action<E, R>
  ::caught_value_action(detail_ptr current, function<R(E&)> routine)
      : _detail
        { std::make_shared<detail>(std::move(current), std::move(routine)) }
  { }

  template<typename ...Value>
  template<typename E, typename R>
  std::shared_ptr<deferred> future<Value...>::caught_value_action<E, R>
  ::operator () (task_context &ctx, promise<R> p)
  {
    return _detail->start(ctx, std::move(p));
  }

  template<typename ...Value>
  template<typename E>
  struct future<Value...>::caught_void_action<E>::detail
  {
    detail_ptr _current;
    promise<> _next;
    function<void(E &)> _routine;
    std::shared_ptr<deferred> _awaited_task;

    detail(detail_ptr current,
           function<void(E &)> routine)
        : _current{std::move(current)}
        , _next{}
        , _routine{std::move(routine)}
    { }

    std::shared_ptr<deferred> start(task_context &ctx, promise<> next)
    {
      _next = std::move(next);
      _current->set_fulfill_action(
          [this] (Value...)
          {
            _next.reject(promise_cancelled());
            _next._detail->deliver();
          }
      );
      _current->set_reject_action(
          [this] (std::exception_ptr p)
          {
            try
            {
              std::rethrow_exception(p);
            }
            catch(E &e)
            {
              _routine(e);
              _next.fulfill();
            }
            catch (...)
            {
              _next.reject_by_exception_ptr(std::current_exception());
            }
            _next._detail->deliver();
          }
      );
      return _current->start(ctx);
//
//      _current.start(_next._detail->_task_context);
//      _current = nullptr;
    }
  };

  template<typename ...Value>
  template<typename E>
  std::shared_ptr<deferred> future<Value...>::caught_void_action<E>
  ::operator () (task_context &ctx, promise<> p)
  {
    return _detail->start(ctx, std::move(p));
  }

  template<typename ...Value>
  template<typename E>
  future<Value...>::caught_void_action<E>
  ::caught_void_action(detail_ptr current,
                       function<void(E &)> routine)
      : _detail
        { std::make_shared<detail>(std::move(current), std::move(routine)) }
  {}


  extern template class future<>;
  extern template class promise<>;
}
