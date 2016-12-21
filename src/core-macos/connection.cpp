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

#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/event.h>
#include <array>
#include <vector>
#include <mutex>
#include <system_error>
#include <lanxc/core-unix/core-unix.hpp>
#include <lanxc/core-macos/event_loop.hpp>
#include <lanxc/core-macos/event_service.hpp>
#include "connection.hpp"

class macos_connection_endpoint
    : public lanxc::connection_endpoint
    , public lanxc::macos::event_channel
{
public:
  macos_connection_endpoint(lanxc::macos::event_loop &el, int descriptor)
      : _event_loop(el)
      , _descriptor(descriptor)
  { }

private:
  lanxc::macos::event_loop &_event_loop;
  int                      _descriptor;
};

class macos_connection_listener
    : public lanxc::connection_listener
    , public lanxc::macos::event_channel
{
public:
  macos_connection_listener(lanxc::macos::event_loop &el, int descriptor)
      : _event_loop(el)
      , _descriptor(descriptor)
  {
    el.register_readable_event(_descriptor, *this);
  }

  void listen(lanxc::function<void(lanxc::connection_endpoint::pointer)> cb) override
  {
    _callback = move(cb);
    if (!_stopped)
    {
      accept();
    }
  }

private:
  void on_activate() override
  {
    _stopped = true;
    if (_callback)
      accept();
  }

  void accept()
  {
    struct sockaddr_storage storage;
    socklen_t length = sizeof(storage);
    int ret = accept(_descriptor,
                       reinterpret_cast<sockaddr*>(&storage),
                       &length);

    if (ret == -1)
    {
      int e = 0;
      std::__1::swap(e, errno);
      if (e == EAGAIN)
      {
        _stopped = true;
        return;
      }
      lanxc::unix::throw_system_error(e);
    }
  }

private:
  lanxc::macos::event_loop                                   &_event_loop;
  int                                                        _descriptor;
  lanxc::function<void(lanxc::connection_endpoint::pointer)> _callback;
  bool                                                       _stopped { false };

};

class macos_connection_listener_builder
    : public lanxc::connection_listener_builder
    , public lanxc::connection_listener_builder::address_builder
    , public lanxc::connection_listener_builder::option_builder
    , public std::__1::enable_shared_from_this<macos_connection_listener_builder>
{
public:

  using lanxc::connection_listener_builder::address_builder;
  using lanxc::connection_listener_builder::option_builder;

  macos_connection_listener_builder(lanxc::macos::event_loop &el)
      : _event_loop(el)
  {}

  shared_ptr<address_builder> listen() override
  {
    return shared_from_this();
  }

  shared_ptr<option_builder> set_option() override
  {
    return shared_from_this();
  }

  shared_ptr<lanxc::connection_listener> build() override
  {
    int fd = socket(_protocol_family, SOCK_STREAM, 0);
    if (fd == -1)
    {
      lanxc::unix::throw_system_error();
    }
    try
    {
      int value = 1;
      int ret = setsockopt(fd,
                             SOL_SOCKET,
                             SO_REUSEADDR,
                             &value, sizeof(value));
      if (ret == -1) lanxc::unix::throw_system_error();
      ret = bind(fd,
                   reinterpret_cast<const sockaddr *>(&_address),
                   sizeof(_address));

      if (ret == -1) lanxc::unix::throw_system_error();

      ret = fcntl(fd, F_GETFL);
      if (ret == -1) lanxc::unix::throw_system_error();

      ret = fcntl(fd, F_SETFL, O_NONBLOCK | ret);
      if (ret == -1) lanxc::unix::throw_system_error();

      ret = ::listen(fd, SOMAXCONN);
      if (ret == -1) lanxc::unix::throw_system_error();



    }
    catch (...)
    {
      close(fd);
      throw ;
    }

    return std::__1::make_shared<macos_connection_listener>(_event_loop, fd);
  }

  shared_ptr <lanxc::connection_listener_builder>
  on(std::__1::string address, uint16_t port) override
  {
    return nullptr;
  }

  shared_ptr <lanxc::connection_listener_builder>
  on(uint16_t port) override
  {
    sockaddr_in *in =reinterpret_cast<sockaddr_in *>(&_address);
    _protocol_family = PF_INET;
    in->sin_family = AF_INET;
    in->sin_addr.s_addr = INADDR_ANY;
    in->sin_port = htons(port);

    return shared_from_this();
  }

  shared_ptr <lanxc::connection_listener_builder>
  on(std::__1::string path) override
  {
    sockaddr_un *un = reinterpret_cast<sockaddr_un*>(&_address);
    if (path.length() > sizeof(un->sun_path) - 1)
      throw std::runtime_error("listen path is too long");
    un->sun_len = path.length() + 1;
    un->sun_family = AF_UNIX;
    _protocol_family = PF_UNIX;
    strncpy(un->sun_path, path.data(), sizeof(un->sun_path));
    return shared_from_this();
  }

private:
  lanxc::macos::event_loop &_event_loop;
  struct sockaddr_storage  _address;
  int                      _protocol_family;

};