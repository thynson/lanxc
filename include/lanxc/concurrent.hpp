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

#ifndef LANXC_CORE_CONCURRENT_HPP
#define LANXC_CORE_CONCURRENT_HPP

#include <chrono>
#include "../../lanxc-core/include/lanxc/function.hpp"

namespace lanxc
{

  class clock_service
  {
  public:
    using std::chrono::steady_clock::time_point;

    virtual ~clock_service() = 0;
    virtual time_point now() = 0;
  };

  class task
  {
  private:
    struct detail;
    std::shared_ptr<detail> m_detail;

  };

  class scheduler
  {
  public:
    void run();

    virtual task dispatch(function<void()> routine);

    virtual void run();

  };


  class synchronous_service : private virtual clock_service
  {
  public:
    virtual void wait_until(time_point tp) = 0;
  };



  class procedure
  {

  };

  class task_service
  {

  };

  class stream_socket
  {

  };

  class datagram_socket
  {

  };

  class named_pipe
  {

  };

  class event_service
  {
  public:
    void wait_for_event();
  };

  class io_service : private virtual clock_service
                   , private virtual synchronous_service
  {

  public:
    class socket_listener_builder
    {

    };

    class datagram_listener_builder
    {

    };

    class socket_builder
    {

    };

    class datagram_builder
    {

    };

    class file_builder
    {

    };

    virtual std::shared_ptr<socket_listener_builder>
    create_socket_listener() = 0;

    virtual std::shared_ptr<datagram_listener_builder>
    create_datagram_listener() = 0;

    virtual std::shared_ptr<socket_listener_builder>
    create_socket() = 0;

    virtual std::shared_ptr<datagram_listener_builder>
    create_datagram() = 0;

    virtual std::shared_ptr<file_builder>
    create_file() = 0;



  };

  class executor_service : private virtual clock_service
                         , private virtual synchronous_service
  {

  public:
  };


}

#endif //LANXC_CORE_CONCURRENT_HPP_HPP
