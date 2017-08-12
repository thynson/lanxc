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

#pragma once

#include <lanxc/function.hpp>
#include <lanxc/config.hpp>

#include <memory>
#include <string>


namespace lanxc
{

  class connection_endpoint
  {
  public:
    using pointer = std::shared_ptr<connection_endpoint>;
  };

  class connection_endpoint_builder
  {
  public:

    std::shared_ptr<connection_endpoint_builder>
    bind(std::string address, std::uint16_t port);


    std::shared_ptr<connection_endpoint> connect(std::string address, std::uint16_t port);

  };

  class LANXC_CORE_EXPORT connection_listener
  {
  public:
    virtual void listen(function<void(connection_endpoint::pointer)> cb) = 0;

    virtual ~connection_listener() = 0;

  };

  class LANXC_CORE_EXPORT connection_listener_builder
  {
  public:
    virtual std::shared_ptr<connection_listener_builder>
    bind(std::string address, std::uint16_t port) = 0;

    virtual std::shared_ptr<connection_listener_builder>
    bind(std::uint16_t port) = 0;

    virtual std::shared_ptr<connection_listener_builder>
    bind(std::string path) = 0;

    virtual std::shared_ptr<connection_listener_builder>
    set_reuse_port(bool enabled) = 0;

    virtual std::shared_ptr<connection_listener_builder>
    set_reuse_address(bool enabled) = 0;

    virtual ~connection_listener_builder() = 0;
    
    virtual std::shared_ptr<connection_listener>
    build(function<void(const connection_endpoint::pointer)> routine) = 0;
  };


  class datagram_endpoint
  {

  };

  class datagram_endpoint_builder
  {

  };

  class LANXC_CORE_EXPORT network_connection_context
  {
  public:
    virtual ~network_connection_context() = 0;


    virtual std::shared_ptr<connection_listener_builder>
    build_connection_listener() = 0;

    virtual std::shared_ptr<connection_endpoint_builder>
    build_connection_endpoint() = 0;

  };

  class LANXC_CORE_EXPORT network_datagram_context
  {
  public:
    virtual ~network_datagram_context() = 0;

    virtual std::shared_ptr<datagram_endpoint_builder>
    create_datagram_endpoint() = 0;
  };


  class LANXC_CORE_EXPORT network_context : public virtual network_connection_context
                                          , public virtual network_datagram_context
  {
  public:

    virtual ~network_context() = 0;

  };
}
