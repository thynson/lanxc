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
#include <lanxc/task.hpp>

#include <exception>
#include <memory>
#include <utility>


namespace lanxc
{
  template<typename ...T>
  class future;

  template<typename ...T>
  class promise;

  template<typename T>
  struct is_future
  { constexpr static bool value = false; };

  template<typename ...T>
  struct is_future<future<T...>>
  { constexpr static bool value = true; };

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
   * @brief Indicate that an operation is not permited for a future
   *
   * If a @a future has been committed or chainned by another future, any
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

  /**
   * @brief The promise for a future
   *
   * A promise guards a process that a future is getting delivered.
   * When a promise is destructed the corresponding future is either fulfilled
   * if #set_result has been called or cancelled. The reason that a future get
   * cancelled is a instance of @a invalid_future, or other value provided by
   * #set_exception_ptr or #set_exception and can be caught as a
   * std::exception_ptr by the future.
   */
  template<typename ...T>
  class promise final
  {
    template<typename ...> friend class future;
  public:

    /**
     * Align the future delivering lifecycle to scope of this instance
     * @param other The original scope guard
     */
    promise(promise &&other) noexcept
        : m_detail(other.m_detail)
        , m_task_monitor{std::move(other.m_task_monitor)}
    { }


    /**
     * @brief Align the future delivering lifecycle to scope of this instance
     * @param other The original scope guard
     */
    promise &operator = (promise &&other) noexcept
    {
      this->~promise();
      new (this) promise(std::move(other));
      return *this;
    }

    promise(const promise&) = delete;
    promise &operator = (const promise &other) = delete;

    /**
     * @brief Fulfill future future with given value
     * @param value The original scope guard
     */
    void set_result(T ...value)
    {
      m_detail->set_result(std::move(value)...);
    }

    /**
     * @brief Cancel the future with given reason @p p
     * @param p The reason that corresponding future is canncelled
     */
    void set_exception_ptr(std::exception_ptr p)
    {
      m_detail->set_exception_ptr(p);
    }

    /**
     * @brief Cancel the future with given reason @p p
     * @tparam E The type of @p e
     * @param e The reason that corresponding future is canncelled
     */
    template<typename E>
    void set_exception(E e)
    {
      m_detail->set_exception_ptr(std::make_exception_ptr<E>(e));
    }

    /**
     * @brief Destructor indicates the end of future delivering
     *
     * When this instance is destructed, the delivering process ends and
     * corresponding future is now either delivered or cancelled
     */
    ~promise()
    {
      if (m_detail == nullptr) return;
      if (!m_detail->m_finisher)
        m_detail->set_default_cancel_error();
      m_detail = nullptr;
    }

  private:

    class detail final : public task
    {
    public:
      function<void(promise<T...>)> m_initiator;
      function<void(std::exception_ptr)> m_error_handler{};
      function<void(T...)> m_fulfill_handler{};
      function<void()> m_finisher{};

      void set_default_cancel_error()
      {
        m_finisher = [this]()
        {
          m_error_handler(std::make_exception_ptr(promise_cancelled()));
        };
      }

      virtual void on_finish() override
      {
        m_initiator = nullptr;
        if (m_finisher) {

          m_finisher();
          m_finisher = nullptr;
        }
      }

      virtual void routine(task_token tm) noexcept override
      {
        m_initiator(promise(this, std::move(tm)));
      }


    public:
      detail(lanxc::function<void(promise<T...>)> f) noexcept
          : m_initiator { std::move(f) }
      { }

      void set_result(T ...value)
      {
        function<void()>(
            std::bind([this](T ...value)
            { m_fulfill_handler(std::move(value)...); }, std::move(value)...)
        ).swap(m_finisher);
      }

      void set_exception_ptr(std::exception_ptr e)
      {
        function<void()>([this, e]()
        {
          m_error_handler(e);
        }).swap(m_finisher);
      }
    };

    static void start(scheduler &s, detail &r)
    {
      if (!r.m_error_handler)
        r.m_error_handler = [] (std::exception_ptr) {};
      if (!r.m_fulfill_handler)
        r.m_fulfill_handler = [] (T...) {};
      s.dispatch(r);
    }

    promise(detail *x, task_token tm) noexcept
        : m_detail { x }
        , m_task_monitor { std::move(tm) }
    { }

    promise(std::nullptr_t) noexcept
        : m_detail { nullptr }
        , m_task_monitor {}
    { }

    detail *m_detail;
    task_token m_task_monitor;

  };


  template<typename ...T>
  class future final
  {
    template<typename ...>
    friend class future;

    using promise_type = promise<T...>;
    using detail = typename promise<T...>::detail;
    static void default_fulfill_handler(T...) {}

    static void default_error_handler(std::exception_ptr) {}

    static function<void(T...)> cascade_fulfill_handler(promise<T...> *p)
    {
      return [p](T...args)
      {
        p->set_result(std::move(args)...);
        *p = promise<T...>(nullptr);
      };
    }

    template<typename Future>
    struct then_future_functor
    {
      struct detail
      {
        future<T...> m_future;
        Future m_next_future{};
        typename Future::promise_type m_next_promise;
        function<Future(T...)> m_function;

        detail(future<T...> f, function<Future(T...)> routine)
            : m_future(std::move(f))
            , m_next_promise{nullptr}
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

      void fulfill(scheduler &s, T ...value)
      {
        m_detail->m_next_future = m_detail->m_function(value...);
        auto &r = *m_detail->m_next_future.m_detail;

        r.m_fulfill_handler
            = Future::cascade_fulfill_handler(&this->m_detail->m_next_promise);

        r.m_error_handler = [this](std::exception_ptr e)
        {
          m_detail->m_next_promise.set_exception_ptr(e);
          m_detail->m_next_promise = typename Future::promise_type(nullptr);
        };
        Future::promise_type::start(s, r);
      }


      void operator () (typename Future::promise_type p)
      {
        auto &s = p.m_task_monitor.get_scheduler();
        m_detail->m_next_promise = std::move(p);

        auto &r = *m_detail->m_future.m_detail;

        r.m_fulfill_handler = [&s, this](T...args)
        {
          this->fulfill(s, std::move(args)...);
        };
        r.m_error_handler = [this](std::exception_ptr e)
        {
          this->m_detail->m_next_promise.set_exception_ptr(e);
          this->m_detail->m_next_promise = typename Future::promise_type(nullptr);
        };
        promise<T...>::start(s, r);
      }
    };


    template<typename Future>
    struct caught_future_functor
    {
      struct detail
      {
        future<T...> m_future;
        Future m_next_future{};
        typename Future::promise_type m_next_promise;
        function<Future(std::exception_ptr)> m_function;

        detail(future<T...> f, function<Future(std::exception_ptr)> routine)
            : m_future(std::move(f))
            , m_next_promise{nullptr}
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

      void caught(scheduler &s, std::exception_ptr e)
      {
        m_detail->m_next_future = m_detail->m_function(e);

        auto &r = *m_detail->m_next_future.m_detail;
        r.m_fulfill_handler
            = Future::cascade_fulfill_handler(&this->m_detail->m_next_promise);

        r.m_error_handler = [this](std::exception_ptr ex)
        {
          m_detail->m_next_promise.set_exception_ptr(ex);
          m_detail->m_next_promise = typename Future::promise_type(nullptr);
        };
        Future::promise_type::start(s, r);
      }

      void operator () (typename Future::promise_type p)
      {
        auto &s = p.m_task_monitor.get_scheduler();
        m_detail->m_next_promise = std::move(p);
        auto &r = *m_detail->m_future.m_detail;

        r.m_fulfill_handler = [this](T...)
        {
          m_detail->m_next_promise = typename Future::promise_type(nullptr);
        };
        r.m_error_handler = [&s, this](std::exception_ptr e)
        {
          caught(s, e);
        };
        promise<T...>::start(s, r);
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
        auto &scheduler = p.m_task_monitor.get_scheduler();
        m_detail->m_next_promise = std::move(p);
        auto &r = *m_detail->m_future.m_detail;
        r.m_fulfill_handler = [this](T ...value)
        {
          m_detail->m_next_promise
                  .set_result(m_detail->m_function(std::move(value)...));
          m_detail->m_next_promise = promise<R>(nullptr);
        };
        r.m_error_handler = [this](std::exception_ptr e)
        {
          m_detail->m_next_promise.set_exception_ptr(e);
          m_detail->m_next_promise = promise<R>(nullptr);
        };

        promise<T...>::start(scheduler, r);
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
        auto &scheduler = p.m_task_monitor.get_scheduler();
        m_detail->m_next_promise = std::move(p);
        auto &r = *m_detail->m_future.m_detail;
        r.m_fulfill_handler = [this](T ...)
        {
          m_detail->m_next_promise = promise<R>(nullptr);
        };

        r.m_error_handler = [this](std::exception_ptr e)
        {
          m_detail->m_next_promise.set_result(m_detail->m_function(e));
          m_detail->m_next_promise = promise<R>(nullptr);
        };

        promise<T...>::start(scheduler, r);
      }
    };



    struct then_void_functor
    {
      struct detail
      {
        lanxc::future<T...> m_future;
        lanxc::promise<> m_next_promise;
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

      void operator () (lanxc::promise<> p)
      {
        auto &scheduler = p.m_task_monitor.get_scheduler();
        m_detail->m_next_promise = std::move(p);
        auto *ptr = m_detail->m_future.m_detail.get();
        ptr->m_fulfill_handler = [this](T ...value)
        {
          m_detail->m_function(std::move(value)...);
          this->m_detail->m_next_promise.set_result();
          this->m_detail->m_next_promise = lanxc::promise<>(nullptr);
        };

        ptr->m_error_handler = [this](std::exception_ptr e)
        {
          this->m_detail->m_next_promise.set_exception_ptr(e);
          this->m_detail->m_next_promise = lanxc::promise<>(nullptr);
        };
        promise<T...>::start(scheduler, *m_detail->m_future.m_detail);
      }
    };

    struct caught_void_functor
    {
      struct detail
      {
        lanxc::future<T...> m_future;
        lanxc::promise<> m_next_promise;
        function<void(std::exception_ptr)> m_function;

        detail(future<T...> &f, function<void(std::exception_ptr)> routine)
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

      void operator () (lanxc::promise<> p)
      {
        auto &scheduler = p.m_task_monitor.get_scheduler();
        m_detail->m_next_promise = std::move(p);
        auto *ptr = m_detail->m_future.m_detail.get();
        ptr->m_fulfill_handler = [this](T ...)
        {
          m_detail->m_next_promise = lanxc::promise<>(nullptr);
        };

        ptr->m_error_handler = [this](std::exception_ptr e)
        {
          m_detail->m_function(e);
          m_detail->m_next_promise.set_result();
          m_detail->m_next_promise = lanxc::promise<>(nullptr);
        };
        promise<T...>::start(scheduler, *m_detail->m_future.m_detail);
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

    void check()
    {
      if (!m_detail
          || m_detail->m_fulfill_handler
          || m_detail->m_error_handler)
        throw invalid_future();
    }


  public:

    future(function<void(lanxc::promise<T...>)> f)
        : m_detail(new typename promise<T...>::detail(std::move(f)))
    { }

    future(future &&f) noexcept = default;

    future &operator = (future &&f) noexcept = default;

    ~future() = default;

    /**
     * @brief Set a callback for receiving the promised value
     * @tparam F Type of the functor
     * @param f The functor
     * @returns Returns a future
     * @throws invalid_future If #then, #caught or #commit has already been
     * called for this future
     *
     * @p f should be able to be called with arguments of `T...`
     *
     * Assuming return value of functor @p f has type of `U`:
     * * if `U` is `void`, this function returns @a future<>;
     * * if `U` is @a future<V...>, this functions returns @a future<V...>
     * * otherwise returns @a future<U>
     */
    template<typename F>
    auto then(F &&f)
    -> typename do_then<typename std::remove_reference<F>::type>::result
    {
      using then_t = do_then<typename std::remove_reference<F>::type>;
      check();
      return typename then_t::result(
          typename then_t::functor(std::move(*this), std::forward<F>(f)));
    }

    /**
     * @brief Set a callback for receiving error
     * @tparam F Type of the functor
     * @param f The functor
     * @returns Returns a future
     * @throws invalid_future If #then, #caught or #commit has already been
     * called for this future
     *
     * @p f should be able to be called with one argument of
     * `std::exception_ptr`
     *
     * Assuming return value of functor @p f has type of `U`:
     * * if `U` is `void`, this function returns @a future<>;
     * * if `U` is @a future<V...>, this functions returns @a future<V...>
     * * otherwise returns @a future<U>
     */
    template<typename F>
    auto caught(F &&f)
    -> typename do_caught<typename std::remove_reference<F>::type>::result
    {
      using caught_t = do_caught<typename std::remove_reference<F>::type>;
      using result = do_caught<typename std::remove_reference<F>::type>;
      using functor = typename do_caught<typename std::remove_reference<F>::type>::functor;
      check();
      return result{functor {std::move(*this), std::forward<F>(f)}};
    }

    /**
     * @brief Starting to resolve this future.
     * @param s The scheduler which will schedule the delivering process of
     * this future
     * @throws invalid_future If #then, #caught or #commit has already been
     * called for this future
     */
    void commit(scheduler &s) &
    {
      check();
      promise<T...>::start(s, *m_detail);
    }


  private:
    constexpr future() noexcept
        : m_detail(nullptr)
    { }
    std::unique_ptr<detail> m_detail;
  };

}

#endif