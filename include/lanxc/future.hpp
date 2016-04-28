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

#ifndef LANXC_FUTURE_HPP_INCLUDED
#define LANXC_FUTURE_HPP_INCLUDED

#include <lanxc/type_traits.hpp>
#include <lanxc/function.hpp>

#include <exception>
#include <memory>
#include <utility>
#include <cassert>


namespace lanxc
{

  /**
   *  Forward declaration
   */
  template<typename ...T>
  class future;

  /**
   * Error and Exception
   */
  class promise_cancelled : public std::exception
  {
  public:


    const char *what() const noexcept override
    {
      return "promise cancelled";
    }
  };

  class invalid_future : public std::exception
  {
  public:
    const char *what() const noexcept override
    {
      return "this future is invalid, possibly because .then(), .caught()"
        " or .commit() was already called for this future.";
    }
  };

  class invalid_promise : public std::exception
  {
  public:
    const char *what() const noexcept override
    {
      return "this promise is invalid, possibly because .set_result(),"
          " .set_exception() or .set_exception_ptr() was called for"
          " this promise.";
    }
  };


  template<typename T>
  struct is_future
  {
    constexpr static bool value = false;
  };
  template<typename ...T>
  struct is_future<future<T...>>
  {
    constexpr static bool value = true;
  };


  template<typename ...T>
  class promise
  {
  public:

    promise(promise &&p) noexcept = default;
    promise &operator = (promise &&p) noexcept = default;

    promise(const promise &p) = delete;
    promise &operator = (const promise &p) = delete;

    /**
     * @brief Fulfill this promise
     * @note After calling this function, this promise becomes invalid
     * @throws invalid_promise if this promise is invalid
     */
    inline void set_result(T ...value)
    {
      if (!m_detail) throw invalid_promise();
      m_detail->set_result(std::move(value)...);
      m_detail = nullptr;
    }

    /**
     * @brief Cancel this promise with an exception as reason
     * @tparam E Type of the exception
     * @param e The exception
     * @note After calling this function, this promise becomes invalid
     * @throws invalid_promise if this promise is invalid
     *
     * This function will cancel this promise and convert @p e into a
     * @a std::exception_ptr which can be caught by the corresponding
     * `future`.
     */
    template<typename E>
    inline void set_exception(E e)
    {
      if (!m_detail) throw invalid_promise();
      m_detail->set_exception_ptr(std::make_exception_ptr(e));
      m_detail = nullptr;
    }

    /**
     * @brief Cancel this promise with an exception pointer as reason
     * @param e The exception pointer
     * @note After calling this function, this promise becomes invalid
     * @throws invalid_promise if this promise is invalid
     *
     * This function will cancel this promise and @p e will be caught
     * by the corresponding `future`.
     */
    inline void set_exception_ptr(std::exception_ptr e)
    {
      if (!m_detail) throw invalid_promise();
      m_detail->set_exception_ptr(e);
      m_detail = nullptr;
    }


  private:
    template<typename...>
    friend class future;

    struct detail
    {
    public:

      detail(function<void(promise<T...>)> initiator) noexcept
          : m_initiator(std::move(initiator))
      { }

      ~detail()
      {
        if (m_error_handler)
          m_error_handler(std::make_exception_ptr(promise_cancelled()));
      }

      detail(const detail&) = delete;
      detail(detail&&) = delete;
      detail &operator = (const detail&) = delete;
      detail &operator = (detail&&) = delete;

      void set_result(T... value)
      {
        if (m_fulfill_handler)
        {
          m_fulfill_handler(std::move(value)...);
          m_fulfill_handler = nullptr;
          m_error_handler = nullptr;
        }
      }

      void set_exception_ptr(std::exception_ptr e) noexcept
      {
        if (m_error_handler)
        {
          m_error_handler(e);
          m_fulfill_handler = nullptr;
          m_error_handler = nullptr;
        }
      }

      function<void(lanxc::promise<T...>)> m_initiator;
      function<void(std::exception_ptr)> m_error_handler;
      function<void(T...)> m_fulfill_handler;
    };

    promise(std::unique_ptr<detail> p)
        : m_detail(std::move(p))
    { }


    static void resolve_promise(std::unique_ptr<detail> p)
    {
      if (!p->m_error_handler)
        p->m_error_handler = [](std::exception_ptr) {};

      if (!p->m_fulfill_handler)
        p->m_fulfill_handler = [](T...){};

      function<void(promise<T...>)> tmp;
      std::swap(p->m_initiator, tmp);
      tmp(promise<T...>(std::move(p)));
    }

    std::unique_ptr<detail> m_detail;
  };

  template<typename ...T>
  class future
  {
    using promise_type = promise<T...>;
    static void default_fulfill_handler(T...) {}

    static void default_error_handler(std::exception_ptr) {}

    static function<void(T...)> cascade_fulfill_handler(promise<T...> *p)
    {
      return [p](T...args) { p->set_result(std::move(args)...); };
    }

    static function<void(T...)> cascade_error_handler(promise<T...> *p)
    {
      return [p](T...args) { std::swap(*p, promise<T...>(nullptr)); };
    }

    template<typename Future>
    struct then_future_functor
    {
      struct detail
      {
        future<T...> m_future;
        typename Future::promise_type m_next_promise;
        function<Future(T...)> m_function;

        detail(future<T...> f, function<Future(T...)> routine)
            : m_future(std::move(f))
            , m_next_promise(nullptr)
            , m_function(std::move(routine))
        {}
      };

      std::unique_ptr<detail> m_detail;

      then_future_functor(future<T...> f, function<Future(T...)> routine)
          : m_detail(new detail(std::move(f), std::move(routine)))
      {};

      then_future_functor(then_future_functor &&o) noexcept
          : m_detail(nullptr)
      {
        std::swap(m_detail, o.m_detail);
      }

      then_future_functor &operator = (then_future_functor &&o) noexcept
      {
        this->~then_future_functor();
        new (this) then_future_functor(std::move(o));
        return *this;
      }

      void fulfill(T ...value)
      {
        Future f = m_detail->m_function(value...);

        f.m_promise->m_fulfill_handler
            = Future::cascade_fulfill_handler(&this->m_detail->m_next_promise);

        f.m_promise->m_error_handler = [this](std::exception_ptr e)
        {
          this->m_detail->m_next_promise.set_exception_ptr(e);
        };
        f.commit();
      }


      void operator () (typename Future::promise_type p)
      {
        m_detail->m_next_promise = std::move(p);
        auto *ptr = m_detail->m_future.m_promise.get();

        ptr->m_fulfill_handler = [this](T...args)
        {
          this->fulfill(std::move(args)...);
        };
        ptr->m_error_handler = [this](std::exception_ptr e)
        {
          this->m_detail->m_next_promise.set_exception_ptr(e);
        };
        m_detail->m_future.commit();
      }
    };


    template<typename Future>
    struct caught_future_functor
    {
      struct detail
      {
        future<T...> m_future;
        typename Future::promise_type m_next_promise;
        function<Future(std::exception_ptr)> m_function;

        detail(future<T...> f, function<Future(std::exception_ptr)> routine)
            : m_future(std::move(f))
            , m_next_promise(nullptr)
            , m_function(std::move(routine))
        { }
      };

      std::unique_ptr<detail> m_detail;

      caught_future_functor(future<T...> f,
                            function<Future(std::exception_ptr)> routine)
          : m_detail(new detail(std::move(f), std::move(routine)))
      { };

      caught_future_functor(caught_future_functor &&o)
          : m_detail(nullptr)
      {
        std::swap(m_detail, o.m_detail);
      }

      caught_future_functor &operator =(caught_future_functor &&o)
      {
        this->~caught_future_functor();
        new (this) caught_future_functor(std::move(o));
        return *this;
      }

      void caught(std::exception_ptr e)
      {
        Future f = m_detail->m_function(e);

        f.m_promise->m_fulfill_handler
            = Future::cascade_fulfill_handler(&this->m_detail->m_next_promise);

        f.m_promise->m_error_handler = [this](std::exception_ptr ex)
        {
          this->m_detail->m_next_promise.set_exception_ptr(ex);
        };
        f.commit();
      }

      void operator () (typename Future::promise_type p)
      {
        m_detail->m_next_promise = std::move(p);
        auto *ptr = m_detail->m_future.m_promise.get();

        ptr->m_fulfill_handler = [this](T...args)
        {
          m_detail->m_next_promise = typename Future::promise_type(nullptr);
        };
        ptr->m_error_handler = [this](std::exception_ptr e)
        {
          caught(e);
        };
        m_detail->m_future.commit();
      }
    };

    template<typename R>
    struct then_value_functor
    {
      struct detail
      {
        future<T...> m_future;
        promise<R> m_next_promise;
        function<R(T...)> m_function;

        detail(future<T...> f, function<R(T...)> routine)
            : m_future(std::move(f))
            , m_next_promise(nullptr)
            , m_function(std::move(routine))
        {}
      };

      std::unique_ptr<detail> m_detail;
      then_value_functor(future<T...> f, function<R(T...)> routine)
          : m_detail(new detail(std::move(f), std::move(routine)))
      { }

      then_value_functor (then_value_functor &&o) noexcept
          : m_detail(nullptr)
      {
        std::swap(m_detail, o.m_detail);
      }

      then_value_functor &operator = (then_value_functor &&o) noexcept
      {
        this->~then_value_functor();
        new (this) then_value_functor(std::move(o));
        return *this;
      }

      void operator () (promise<R> p)
      {
        m_detail->m_next_promise = std::move(p);
        auto *ptr = m_detail->m_future.m_promise.get();
        ptr->m_fulfill_handler = [this](T ...value)
        {
          m_detail->m_next_promise
                  .set_result(m_detail->m_function(std::move(value)...));
        };
        ptr->m_error_handler = [this](std::exception_ptr e)
        {
          this->m_detail->m_next_promise.set_exception_ptr(e);
        };

        m_detail->m_future.commit();
      }

    };

    template<typename R>
    struct caught_value_functor
    {
      struct detail
      {
        future<T...> m_future;
        promise<R> m_next_promise;
        function<R(std::exception_ptr)> m_function;

        detail(future<T...> f, function<R(std::exception_ptr)> routine)
            : m_future(std::move(f))
            , m_next_promise(nullptr)
            , m_function(std::move(routine))
        {}
      };

      std::unique_ptr<detail> m_detail;
      caught_value_functor(future<T...> f,
                           function<R(std::exception_ptr)> routine)
          : m_detail(new detail(std::move(f), std::move(routine)))
      { }

      caught_value_functor(caught_value_functor &&o) noexcept
          : m_detail(nullptr)
      {
        std::swap(m_detail, o.m_detail);
      }
      caught_value_functor &operator = (caught_value_functor &&o) noexcept
      {
        this->~caught_value_functor();
        new (this) caught_value_functor(std::move(o));
        return *this;
      }

      void operator () (promise<R> p)
      {
        m_detail->m_next_promise = std::move(p);
        auto *ptr = m_detail->m_future.m_promise.get();
        ptr->m_fulfill_handler = [this](T ...value)
        {
          m_detail->m_next_promise = promise<R>(nullptr);
        };

        ptr->m_error_handler = [this](std::exception_ptr e)
        {
          m_detail->m_next_promise.set_result(m_detail->m_function(e));
        };

        m_detail->m_future.commit();
      }
    };



    struct then_void_functor
    {
      struct detail
      {
        future<T...> m_future;
        promise<> m_next_promise;
        function<void(T...)> m_function;

        detail(future<T...> f, function<void(T...)> routine)
            : m_future(std::move(f))
            , m_next_promise(nullptr)
            , m_function(std::move(routine))
        {}
      };

      std::unique_ptr<detail> m_detail;
      then_void_functor(future<T...> f, function<void(T...)> routine)
          : m_detail(new detail(std::move(f), std::move(routine)))
      { }

      then_void_functor(then_void_functor &&o) noexcept
          : m_detail(nullptr)
      {
        std::swap(m_detail, o.m_detail);
      }
      then_void_functor &operator = (then_void_functor &&o) noexcept
      {
        this->~then_void_functor();
        new (this) then_void_functor(std::move(o));
        return *this;
      }

      void operator () (promise<> p)
      {
        m_detail->m_next_promise = std::move(p);
        auto *ptr = m_detail->m_future.m_promise.get();
        ptr->m_fulfill_handler = [this](T ...value)
        {
          m_detail->m_function(std::move(value)...);
          this->m_detail->m_next_promise.set_result();
        };

        ptr->m_error_handler = [this](std::exception_ptr e)
        {
          this->m_detail->m_next_promise.set_exception_ptr(e);
        };
        m_detail->m_future.commit();
      }
    };

    struct caught_void_functor
    {
      struct detail
      {
        future<T...> m_future;
        promise<> m_next_promise;
        function<void(std::exception_ptr)> m_function;

        detail(future<T...> f, function<void(std::exception_ptr)> routine)
            : m_future(std::move(f))
            , m_next_promise(nullptr)
            , m_function(std::move(routine))
        {}
      };

      std::unique_ptr<detail> m_detail;
      caught_void_functor(future<T...> f,
                          function<void(std::exception_ptr)> routine)
          : m_detail(new detail(std::move(f), std::move(routine)))
      { }

      void operator () (promise<> p)
      {
        m_detail->m_next_promise = std::move(p);
        auto *ptr = m_detail->m_future.m_promise.get();
        ptr->m_fulfill_handler = [this](T ...value)
        {
          m_detail->m_next_promise = promise<>(nullptr);
        };

        ptr->m_error_handler = [this](std::exception_ptr e)
        {
          m_detail->m_function(e);
          m_detail->m_next_promise.set_result();
        };
        m_detail->m_future.commit();
      }
    };

    template<typename U>
    using is_void_sfinae = typename std::enable_if
        <std::is_void<U>::value>::type;

    template<typename U>
    using is_future_sfinae = typename std::enable_if
        <is_future<U>::value>::type;

    template<typename F>
    using then_result_type = typename result_of<F(T...)>::type;

    template<typename F>
    using caught_result_type = typename result_of
        <F(std::exception_ptr)>::type;

    template<typename F,typename=void>
    struct do_then
    {
      using functor = then_value_functor<then_result_type<F>>;
      using result = future<then_result_type<F>>;
    };

    template<typename F>
    struct do_then<F, is_void_sfinae<then_result_type<F>>>
    {
      using functor = then_void_functor;
      using result = future<>;
    };

    template<typename F>
    struct do_then<F, is_future_sfinae<then_result_type<F>>>
    {
      using functor = then_future_functor<typename result_of<F(T...)>::type>;
      using result = typename result_of<F(T...)>::type;
    };



    template<typename F,typename=void>
    struct do_caught
    {
      using functor = caught_value_functor<caught_result_type<F>>;
      using result = future<caught_result_type<F>>;
    };

    template<typename F>
    struct do_caught<F, is_void_sfinae<caught_result_type<F>>>
    {
      using functor = caught_void_functor;
      using result = future<>;
    };

    template<typename F>
    struct do_caught<F, is_future_sfinae<caught_result_type<F>>>
    {
      using functor = caught_future_functor
          <typename result_of<F(std::exception_ptr)>::type>;
      using result = typename result_of<F(std::exception_ptr)>::type;
    };


  public:

    future(function<void(lanxc::promise<T...>)> initiator)
        : m_promise(new typename promise<T...>::detail(std::move(initiator)))
    { }

    future(future &&f) noexcept = default;

    future &operator = (future &&f) noexcept = default;

    ~future() = default;

    /**
     * @brief Set a callback for receiving the promised value
     * @tparam F Type of the functor
     * @param f The functor
     * @returns Returns a future
     * @note After calling this function, this future becomes invalid
     * @throws invalid_future if this future is invalid
     *
     * @p f should be able to be called with arguments of `T...`
     *
     * Assuming return value of functor @p f has type of `U`
     * * if `U` is `void`, this function returns `future<>`;
     * * if `U` is `future<V...>`, this functions returns `future<V...>`
     * * otherwise returns `future<U>`
     */
    template<typename F>
    auto then(F &&f)
      -> typename do_then<typename std::remove_reference<F>::type>::result
    {
      using then_t = do_then<typename std::remove_reference<F>::type>;
      if (!m_promise) throw invalid_future();
      return typename then_t::result(
          typename then_t::functor(std::move(*this),
                                   std::forward<F>(f)));
    }

    /**
     * @brief Set a callback for receiving error
     * @tparam F Type of the functor
     * @param f The functor
     * @returns Returns a future
     * @note After calling this function, this future becomes invalid
     * @throws invalid_future if this future is invalid
     *
     * @p f should be able to be called with one arguments of
     * `std::exception_ptr`
     *
     * Assuming return value of functor @p f has type of `U`
     * * if `U` is `void`, this function returns `future<>`;
     * * if `U` is `future<V...>`, this functions returns `future<V...>`
     * * otherwise returns `future<U>`
     */
    template<typename F>
    auto caught(F &&f)
      -> typename do_caught<typename std::remove_reference<F>::type>::result
    {
      using caught_t = do_caught<typename std::remove_reference<F>::type>;
      if (!m_promise) throw invalid_future();
      return typename caught_t::result(
          typename caught_t::functor(std::move(*this),
                                     std::forward<F>(f)));
    }

    /**
     * @brief Starting to resolve this future.
     * @note After calling this method this future becomes invalid
     * @throws invalid_future if this future is invalid
     */
    void commit()
    {
      if (!m_promise) throw invalid_future();
      promise<T...>::resolve_promise(std::move(m_promise));
    }

  private:

    future() noexcept
        : m_promise(nullptr)
    { }

    template<typename...>
    friend class promise;

    template<typename ...>
    friend class future;

    std::unique_ptr<typename promise<T...>::detail> m_promise;
  };
}

#endif
