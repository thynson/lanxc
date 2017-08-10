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

#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/fcntl.h>
#include <sys/un.h>

#include <cerrno>

#include <lanxc-applism/concrete_network_connection.hpp>

#include "connection.hpp"

namespace
{
  using namespace lanxc;
  using namespace lanxc::applism;

  class macos_connection_endpoint
      : public concrete_event_source
        , public lanxc::connection_endpoint
        , public readable_event_channel
        , public writable_event_channel
        , public error_event_channel
  {
  public:
    class builder;
    macos_connection_endpoint(event_service &, int descriptor);

    void on_readable(intptr_t data, std::uint32_t flags) override;

    void on_writable(intptr_t data, std::uint32_t flags) override;

    void on_error(intptr_t data, std::uint32_t flags) override;

  private:

    int _descriptor;
  };

  class macos_connection_endpoint::builder
      : public lanxc::connection_endpoint_builder
  {
    friend class macos_connection_endpoint;
  public:
    builder(lanxc::applism::event_loop &el);


  };

  class macos_connection_listener
      : public concrete_event_source
        , public lanxc::connection_listener
        , public lanxc::applism::readable_event_channel
  {
  public:
    class builder
        : public lanxc::connection_listener_builder
          , public lanxc::connection_listener_builder::address_builder
          , public lanxc::connection_listener_builder::option_builder
          , public std::enable_shared_from_this<builder>
    {
      friend class macos_connection_listener;
    public:

      using lanxc::connection_listener_builder::address_builder;
      using lanxc::connection_listener_builder::option_builder;

      builder(lanxc::applism::event_loop &el);

      std::shared_ptr<address_builder> listen() override;

      std::shared_ptr<option_builder> set_option() override;

      std::shared_ptr<connection_listener> build() override;

      std::shared_ptr<connection_listener_builder>
      on(std::string address, uint16_t port) override;

      std::shared_ptr<connection_listener_builder>
      on(uint16_t port) override;

      std::shared_ptr<connection_listener_builder>
      on(std::string path) override;

    private:

      unixy::file_descriptor create_socket_descriptor();

      lanxc::applism::event_loop &_event_loop;
      struct sockaddr_storage  _address;
      int                      _protocol_family;

    };

    macos_connection_listener(builder &builder);

    void
    listen(lanxc::function<void(lanxc::connection_endpoint::pointer)> cb)
    override;

  private:

    void on_readable(std::intptr_t data, std::uint32_t flags) override;

    void accept();

  private:
    lanxc::applism::event_loop &_event_loop;
    lanxc::function<void(lanxc::connection_endpoint::pointer)> _callback;
    bool _stopped { false };

  };

  macos_connection_endpoint::macos_connection_endpoint(event_service &el,
                                                       int descriptor )
      : concrete_event_source(unixy::file_descriptor(descriptor))
      , readable_event_channel(el)
      , writable_event_channel(el)
      , error_event_channel(el)
  {
  }

  void macos_connection_endpoint::on_readable(intptr_t, std::uint32_t)
  {
  }

  void macos_connection_endpoint::on_writable(intptr_t, std::uint32_t)
  {
  }

  void macos_connection_endpoint::on_error(intptr_t, std::uint32_t)
  {
  }

  macos_connection_listener::macos_connection_listener(builder &builder)
      : concrete_event_source(builder.create_socket_descriptor())
      , readable_event_channel(builder._event_loop)
      , _event_loop(builder._event_loop)
  {

  }

  void macos_connection_listener::listen(
      lanxc::function<void(lanxc::connection_endpoint::pointer)> cb)
  {
    _callback = std::move(cb);
    if (!_stopped)
    {
      accept();
    }
  }

  void macos_connection_listener::on_readable(std::intptr_t, std::uint32_t)
  {
    _stopped = true;
    if (_callback)
      accept();
  }

  void macos_connection_listener::accept()
  {
    struct sockaddr_storage storage;
    socklen_t length = sizeof(storage);
    int ret = ::accept(get_file_descriptor(),
                       reinterpret_cast<sockaddr*>(&storage),
                       &length);

    if (ret == -1)
    {
      int e = 0;
      std::swap(e, errno);
      if (e == EAGAIN)
      {
        _stopped = true;
        return;
      }
      lanxc::unixy::throw_system_error(e);
    }
  }
  std::shared_ptr<connection_listener_builder::address_builder>
  macos_connection_listener::builder::listen()
  {
    return shared_from_this();
  }

  std::shared_ptr<connection_listener_builder::option_builder>
  macos_connection_listener::builder::set_option()
  {
    return shared_from_this();
  }

  unixy::file_descriptor
  macos_connection_listener::builder::create_socket_descriptor()
  {

    unixy::file_descriptor fd {socket(_protocol_family, SOCK_STREAM, 0)};
    if (!fd)
      lanxc::unixy::throw_system_error();

    int value = 1;
    int ret = setsockopt(fd,
                         SOL_SOCKET,
                         SO_REUSEADDR,
                         &value, sizeof(value));
    if (ret == -1) lanxc::unixy::throw_system_error();
    ret = bind(fd,
               reinterpret_cast<const sockaddr *>(&_address),
               sizeof(_address));

    if (ret == -1) lanxc::unixy::throw_system_error();

    ret = fcntl(fd, F_GETFL);
    if (ret == -1) lanxc::unixy::throw_system_error();

    ret = fcntl(fd, F_SETFD, ret|O_NONBLOCK);

    if (ret == -1) lanxc::unixy::throw_system_error();

    ret = ::listen(fd, SOMAXCONN);
    if (ret == -1) lanxc::unixy::throw_system_error();
    return fd;
  }

  std::shared_ptr<lanxc::connection_listener>
  macos_connection_listener::builder::build()
  {
    return std::make_shared<macos_connection_listener>(*this);
  }

  std::shared_ptr<lanxc::connection_listener_builder>
  macos_connection_listener::builder::on(
      std::string address, uint16_t port)
  {
    sockaddr_in *in =reinterpret_cast<sockaddr_in *>(&_address);
    _protocol_family = PF_INET;
    in->sin_family = AF_INET;
    in->sin_addr.s_addr = INADDR_ANY;
    if (!inet_aton(address.data(), &in->sin_addr))
    {
      throw std::runtime_error("invalid inet address");
    }
    in->sin_port = htons(port);

    return shared_from_this();
  }

  std::shared_ptr <lanxc::connection_listener_builder>
  macos_connection_listener::builder::on(uint16_t port)
  {
    sockaddr_in *in =reinterpret_cast<sockaddr_in *>(&_address);
    _protocol_family = PF_INET;
    in->sin_family = AF_INET;
    in->sin_addr.s_addr = INADDR_ANY;
    in->sin_port = htons(port);

    return shared_from_this();
  }

  std::shared_ptr <lanxc::connection_listener_builder>
  macos_connection_listener::builder::on(std::string path)
  {
    sockaddr_un *un = reinterpret_cast<sockaddr_un*>(&_address);
    if (path.length() > sizeof(un->sun_path) - 1)
      throw std::runtime_error("listen path is too long");
    un->sun_len = std::uint8_t(path.length() + 1);
    un->sun_family = AF_UNIX;
    _protocol_family = PF_UNIX;
    strncpy(un->sun_path, path.data(), sizeof(un->sun_path));
    return shared_from_this();
  }

}

lanxc::applism::network_connection_context::~network_connection_context() = default;

std::shared_ptr<connection_endpoint_builder>
lanxc::applism::network_connection_context::create_connection_endpoint()
{
  return std::make_shared<macos_connection_listener::builder>(this);
}
