/*
 * Copyright (C) 2016 LAN Xingcan
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

#ifndef LANXC_EXECUTOR_CONTEXT_HPP
#define LANXC_EXECUTOR_CONTEXT_HPP

#include <lanxc/function.hpp>

#include <exception>
#include <vector>

namespace lanxc
{


  class executor_context
  {
  public:
    virtual void execute(lanxc::function<void()> routine) = 0;
  };

  template<typename ...Value>
  class promise;

  template<typename ...Value>
  class future;

  /**
   * @brief Indicate that the future has been cancelled without a reason
   *
   * If a @a promise is destructed when neither promise<T...>::set_result,
   * promise<T...>::set_exception nor promise<T...>::set_exception_ptr has
   * been called. The corresponding future will receive an exception pointer
   * wrapping an instance of this class.
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
      return "this future is invalid, possibly because .then(), .caught()"
          " or .commit() was already called for this future.";
    }
  };

  template<>
  class promise<void>
  {

  };

  template<>
  class future<void>
  {

  };


  template<typename ...Value>
  class promise
  {
    template<typename ...> friend class future;
  public:

    promise(promise &&) = default;
    promise &operator = (promise &&) = default;

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

//    struct functor
//    {
//      promise<Value ...> _self;
//      functor(promise<Value ...> self) : _self(std::move(self)) {}
//
//      functor(const functor &) = delete;
//      functor(functor &&) = default;
//      functor &operator = (const functor &) = delete;
//      functor &operator = (functor &&) = default;
//
//      void operator () ()
//      {
//        auto &f = _self._detail->_start;
//        f(std::move(_self));
//      }
//    };

    promise(function<void(promise<Value...>)> initiator)
        : _detail{std::make_shared<detail>(std::move(initiator))}
    { }

    promise(std::shared_ptr<detail> x)
        : _detail(std::move(x))
    {
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
      ctx->execute(
          [_detail]
          {
            auto &f = _detail->_start;
            f(promise(_detail));
          }
      );
    }

    template<typename ...T>
    struct pack;

    template<typename T>
    struct store
    {
      typename std::decay<T>::type _x;
      store(T &&x): _x(std::forward<T>(x))
      { }
    };

    template<typename T, typename ...R>
    struct pack<T, R...> : store<T>, pack<R...>
    {
      pack(T &&x, R &&...y)
          : store(std::forward<T>(x))
          , pack<R...>(std::forward<R>(y)...)
      { }

      void operator () ()
      {
        pack<> &self = *this;
        self._detail._start(
            static_cast<T>(static_cast<store<T>&>(*this)._x),
            static_cast<R>(static_cast<store<R>&>(*this)._x)...);
      }
    };

    template<>
    struct pack<>
    {
      detail &_detail;
      pack(detail &x)
        : _detail(x)
      {

      }
    };

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
        function<void()>(std::bind([this](Value ...value)
                  { _fulfill(std::move(value)...); },
                  std::move(result)...)).swap(_finish);
      }

      void set_exception_ptr(std::exception_ptr e)
      {
        function<void()>([this, e] { _reject(e); }).swap(_finish);
      }
    };

    std::shared_ptr<detail> _detail;
  };

  template<typename ...Value>
  class future
  {
  public:

    future(function<void(promise<Value...>)> initiator)
        : _promise(std::move(initiator))
    { }

    void commit(executor_context &ctx)
    {
    }


    template<typename R>
    future<R> then(function<R(Value...)> done)
    {
      return nullptr;
    }

    future<> then(function<void(Value...)> done)
    {
      return nullptr;
    }

    template<typename ...R>
    future<R...> then(function<future<R...>(Value...)> done)
    {
      return nullptr;
    }

    future<> then(function<future<>(Value...)> done)
    {
      return nullptr;
    }

    template<typename E, typename R>
    future<R> caught(function<R(E&)> done)
    {
      return nullptr;
    }

    template<typename E>
    future<> caught(function<void(E&)> done)
    {
      return nullptr;
    }

    template<typename E, typename ...R>
    future<R...> caught(function<future<R...>(E&)> done)
    {
      return nullptr;
    }

    template<typename E>
    future<> caught(function<future<>(E&)> done)
    {
      return nullptr;
    }

    void start(executor_context &);

  private:

    template<typename ...R>
    struct then_future_action
    {
      struct detail
      {
        future<Value...> _current;
        promise<R...> _next;
        function<future<R...>(Value...)> _routine;

        detail(future<Value...> current,
               promise<> next,
               function<future<R...>(Value...)> routine)
            : _current(std::move(current))
            , _next(std::move(next))
            , _routine(std::move(routine))
        { }

        void start(promise<> next)
        {
          _next = std::move(next);
          _current._promise.set_fulfill_action(
              [this] (Value ...value)
              {
                try
                {
                  auto f = _routine(std::move(value)...);

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

      void operator () (promise<> next)
      {
        _detail->start(std::move(next));
      }
    };

    struct then_void_action
    {
      struct detail
      {
        future<Value...> _current;
        promise<> _next;
        function<void(Value...)> _routine;

        detail(future<Value...> current,
               promise<> next,
               function<void(Value...)> routine)
          : _current(std::move(current))
          , _next(std::move(next))
          , _routine(std::move(routine))
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
    };


    promise<Value...> _promise;

  };


}



