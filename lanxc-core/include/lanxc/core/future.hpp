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

#include "executor_context.hpp"
#include <lanxc/function.hpp>
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
  class promise_cancelled : public std::exception
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
  class invalid_future : public std::exception
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
  class promise
  {
    template<typename ...> friend class future;
  public:

    promise(promise &&) noexcept = default;
    promise &operator = (promise &&) noexcept = default;

    promise(const promise &) = delete;
    promise &operator = (const promise &) = default;

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

    struct detail
    {
      function<void(promise<Value...>)> _start;
      function<void(Value...)> _fulfill;
      function<void(std::exception_ptr)> _reject;
      function<void()> _finish{};
      executor_context *_executor_context;

      detail(function<void(promise<Value...>)> routine,
             function<void(Value...)> fulfill_action,
             function<void(std::exception_ptr)> reject_action)
          : _start(std::move(routine))
          , _fulfill{std::move(fulfill_action)}
          , _reject{std::move(reject_action)}
          , _finish
              {
                  [this]
                  { _reject(std::make_exception_ptr(promise_cancelled())); }
              }
          , _executor_context{nullptr}
      { }

      ~detail()
      {
        _finish();
      }

      void set_result(Value ...result)
      {
        _finish = std::bind([this](Value ...value)
                            { _fulfill(std::move(value)...);},
                            std::move(result)...);
      }

      void set_exception_ptr(std::exception_ptr e)
      {
        _finish = [this, e] { _reject(e); };
      }
    };

    std::shared_ptr<detail> _detail;

    promise(function<void(promise<Value...>)> initiator)
        : _detail{std::make_shared<detail>(std::move(initiator),
                                           [](Value...) {},
                                           [](std::exception_ptr){})}
    { }

    promise(std::shared_ptr<detail> x)
        : _detail(std::move(x))
    {
    }

    promise(std::nullptr_t = nullptr)
        : _detail {nullptr}
    {

    }

    promise &operator = (std::nullptr_t)
    {
      _detail = nullptr;
      return *this;
    }

    promise &set_fulfill_action(function<void(Value...)> action)
    {
      _detail->_fulfill.swap(action);
      return *this;
    }

    promise &set_reject_action(function<void(std::exception_ptr)> action)
    {
      _detail->_reject.swap(action);
      return *this;
    }

    void start(executor_context *ctx)
    {
      _detail->_executor_context = ctx;
      auto d = _detail;
      ctx->dispatch(
          [d]
          {
            auto &f= d->_start;
            f(promise(d));
          }
      );
    }
  };

  template<typename ...Value>
  class future
  {

    template<typename F, typename R = typename result_of<F(Value...)>::type>
    struct then_type
    {
      using future_type = future<R>;
      static future_type
      construct_future(future self, function<R (Value...)> f)
      {
        return future_type
            {
                then_value_action<R>(std::move(self), std::move(f))
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
                then_void_action(std::move(self), std::move(f))
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
                then_future_action<V...>(std::move(self), std::move(f))
            };
      }
    };

    template<typename E, typename F, typename R=typename result_of<F(E&)>::type>
    struct caught_type
    {
      using future_type = future<R>;

      static future_type
      construct_future(future self, function<R(E&)> f)
      {
        return future_type
            {
                caught_value_action<E,R> (std::move(self), std::move(f))
            };
      }
    };
    template<typename E, typename F>
    struct caught_type<E, F, void>
    {
      using future_type = future<>;

      static future_type
      construct_future(future self, lanxc::function<void(E&)> f)
      {
        return future_type
            {
                caught_void_action<E> (std::move(self), std::move(f))
            };
      }
    };

    template<typename E, typename F, typename ...V>
    struct caught_type<E, F, future<V...>>
    {
      using future_type = future<V...>;

      static future_type
      construct_future(future self, function<future_type(E&)> f)
      {
        return future_type
            {
                caught_future_action<E, V...> (std::move(self), std::move(f))
            };
      }
    };

  public:

    /**
     * @brief Construct a future with a functor
     * @param initiator the functor that will fulfill the promise
     */
    future(function<void(promise<Value...>)> initiator)
        : _promise(std::move(initiator))
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
    typename caught_type<E, F>::future_type
    caught(F &&f)
    {
      return caught_type<E, F>::construct_future(std::move(*this),
                                                 std::move(f));
    };

    /**
     * @brief Resolve this future within an executor
     * @param ctx The executor
     */
    void start(executor_context &ctx)
    {
      _promise.start(&ctx);
    }

  private:

    template<typename ...R>
    struct then_future_action
    {
      struct detail
      {
        future _current;
        promise<R...> _next;
        function<future<R...>(Value...)> _routine;

        detail(future current,
               function<future<R...>(Value...)> routine)
            : _current{std::move(current)}
            , _next{}
            , _routine{std::move(routine)}
        { }

        void start(promise<R...> next)
        {
          _next = std::move(next);
          _current._promise.set_fulfill_action(
              [this] (Value ...value)
              {
                try
                {
                  auto f = _routine(std::move(value)...);

                  f.then(
                      function<void(R...)> {
                          [this](R ... r)
                          {
                            _next.fulfill(std::move(r)...);
                            _next = nullptr;
                          }
                      }
                  );
                  f.start(*_next._detail->_executor_context);
                }
                catch (...)
                {
                  _next.reject_by_exception_ptr(std::current_exception());
                }
              }
          );

          _current._promise.set_reject_action(
              [this] (std::exception_ptr e)
              {
                _next.reject_by_exception_ptr(e);
                _next = nullptr;
              }
          );

          _current.start(*_next._detail->_executor_context);

        }
      };

      std::shared_ptr<detail> _detail;

      void operator () (promise<R...> next)
      {
        _detail->start(std::move(next));
      }

      then_future_action(
          future current,
          function<future<R...>(Value...)> routine)
          : _detail{std::make_shared<detail>(
          std::move(current), std::move(routine))}
      { }
    };

    template<typename R>
    struct then_value_action
    {
      struct detail
      {
        future _current;
        promise<R> _next;
        function<R(Value...)> _routine;

        detail(future current,
               function<R(Value...)> routine)
            : _current{std::move(current)}
            , _next{}
            , _routine{std::move(routine)}
        { }

        void start(promise<R> next)
        {
          _next = std::move(next);
          _current._promise.set_fulfill_action(
              [this] (Value ...value)
              {
                try
                {
                  _next.fulfill(_routine(std::forward<Value>(value)...));
                }
                catch(...)
                {
                  _next.reject_by_exception_ptr(std::current_exception());
                }

                _next = nullptr;
              }
          );
          _current._promise.set_reject_action(
              [this] (std::exception_ptr e) {
                _next.reject_by_exception_ptr(e);
                _next = nullptr;
              }
          );
          _current.start(*next._detail->_executor_context);
        }
      };

      std::shared_ptr<detail> _detail;

      void operator () (promise<R> next)
      {
        _detail->start(std::move(next));
      }

      then_value_action(future current,
                        function<R(Value...)> routine)
          : _detail
          { std::make_shared<detail>(std::move(current), std::move(routine)) }
      { }
    };


    struct then_void_action
    {
      struct detail
      {
        future _current;
        promise<> _next;
        function<void(Value...)> _routine;

        detail(future current,
               function<void(Value...)> routine)
            : _current{std::move(current)}
            , _next{}
            , _routine{std::move(routine)}
        { }

        void start(promise<> next)
        {
          _next = std::move(next);
          _current._promise.set_fulfill_action(
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
                _next = nullptr;
              }
          );

          _current._promise.set_reject_action(
              [this] (std::exception_ptr e)
              {
                _next.reject_by_exception_ptr(e);
                _next = nullptr;
              }
          );

          _current.start(*next._detail->_executor_context);

        }
      };

      std::shared_ptr<detail> _detail;

      void operator () (promise<> next)
      {
        _detail->start(std::move(next));
      }

      then_void_action(
          future<Value...> current,
          function<void(Value...)> done
      ) : _detail
          { std::make_shared<detail>(std::move(current), std::move (done))}
      {

      }
    };


    template<typename E, typename ...R>
    struct caught_future_action
    {
      struct detail
      {
        future _current;
        promise<R...> _next;
        function<future<R...>(E &)> _routine;

        detail(future current,
               function<future<R...>(E &)> routine)
            : _current{std::move(current)}
            , _next{}
            , _routine{std::move(routine)}
        { }

        void start(promise<R...> next)
        {
          _next = std::move(next);
          _current._promise.set_fulfill_action(
              [this] (Value ...)
              {
                _next.reject(promise_cancelled());
                _next = nullptr;
              }
          );

          _current._promise.set_reject_action(
              [this] (std::exception_ptr p)
              {
                try
                {
                  std::rethrow_exception(p);
                }
                catch (E &e)
                {
                  auto f = _routine(e);

                  f.then(
                      [this] (R ... r)
                      {
                        _next.fulfill(std::move(r)...);
                        _next = nullptr;
                      }
                  );
                  f.start(*_next._detail->_executor_context);
                }
                catch (...)
                {
                  _next.reject_by_exception_ptr(std::current_exception());
                  _next = nullptr;
                }
              }
          );

          _current.start(*_next._detail->_executor_context);

        }
      };

      std::shared_ptr<detail> _detail;
      caught_future_action(future current,
                           function<future<R...>(E&)> routine)
          : _detail(
          std::make_shared<detail>(std::move(current), std::move (routine)))
      {}
      void operator () (promise<R...> p)
      {
        _detail->start(std::move(p));
      }
    };

    template<typename E, typename R>
    struct caught_value_action
    {
      struct detail
      {
        future<Value...> _current;
        promise<R> _next;
        function<R(E &)> _routine;

        detail(future<Value...> current,
               function<R(E &)> routine)
            : _current{std::move(current)}
            , _next{}
            , _routine{std::move(routine)}
        { }

        void start(promise<R> next)
        {
          _next = std::move(next);
          _current._promise.set_fulfill_action(
              [this] (Value ...)
              {
                _next.reject(promise_cancelled());
                _next = nullptr;
              }
          );
          _current._promise.set_reject_action(
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

                _next = nullptr;
              }
          );
          _current.start(*next._detail->_executor_context);
        }
      };
      std::shared_ptr<detail> _detail;
      caught_value_action(future current,
                          function<R(E &)> routine)
          : _detail(
          std::make_shared<detail>(std::move(current), std::move(routine)))
      {}

      void operator () (promise<R> p)
      {
        _detail->start(std::move(p));
      }
    };

    template<typename E>
    struct caught_void_action
    {
      struct detail
      {
        future<Value...> _current;
        promise<> _next;
        function<void(E &)> _routine;

        detail(future<Value...> current,
               function<void(E &)> routine)
            : _current{std::move(current)}
            , _next{}
            , _routine{std::move(routine)}
        { }

        void start(promise<> next)
        {
          _next = std::move(next);
          _current._promise.set_fulfill_action(
              [this] (Value...)
              {
                _next.reject(promise_cancelled());
                _next = nullptr;
              }
          );

          _current._promise.set_reject_action(
              [this] (std::exception_ptr p)
              {
                try
                {
                  std::rethrow_exception(p);
                }
                catch(E &e)
                {
                  _next.fulfill();
                }
                catch (...)
                {
                  _next.reject_by_exception_ptr(std::current_exception());
                }
                _next = nullptr;
              }
          );

          _current.start(*next._detail->_executor_context);

        }
      };

      void operator () (promise<> p)
      {
        _detail->start(std::move(p));
      }

      std::shared_ptr<detail> _detail;
      caught_void_action(future current,
                         function<void(E &)> routine)
          : _detail(
          std::make_shared<detail>(std::move(current), std::move(routine)))
      {}
    };
    promise<Value...> _promise;
  };


}
