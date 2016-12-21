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

#include <lanxc/core-macos/event_loop.hpp>
#include <lanxc/core/network_context.hpp>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <system_error>

struct stream_listener_options
{
  stream_listener_options(lanxc::network_context &ctx)
      : _context (ctx)
  {
  }
  lanxc::network_context &_context;
  struct sockaddr_storage storage;

};


class macos_stream_listener_builder
    : public lanxc::connection_listener_builder
    , public std::enable_shared_from_this<macos_stream_listener_builder>
{
public:
  macos_stream_listener_builder(lanxc::network_context &ctx)
      : _options(new stream_listener_options(ctx))
  { }

  std::shared_ptr<address_builder> listen() override
  {
    return nullptr;
  }

  std::shared_ptr<option_builder> set_option() override
  {
    return nullptr;
  }

  lanxc::connection_listener build() override
  {
    if (_options == nullptr) throw std::runtime_error("builder invalid");
    int protocol_family = 0;

    switch(_options->storage.ss_family) {
      case AF_INET:
        protocol_family = PF_INET;
        break;
      case AF_INET6:
        protocol_family = PF_INET6;
        break;
      default:
        throw std::runtime_error("unknown or uninitialized address family");
    }
    int fd = 0;
    try
    {
      fd = ::socket(protocol_family, SOCK_STREAM, 0);
      socklen_t length = sizeof (_options->storage);
      int ret = ::bind(fd, reinterpret_cast<sockaddr*>(&_options->storage),
          length);
      if (ret == -1)
      {
        int e = 0;
        std::swap(e, errno);
        throw std::system_error(std::error_code(e));
      }

    }
    catch (...)
    {
      ::close(fd);
      throw;
    }
  }

private:
  std::unique_ptr<stream_listener_options> _options;

};

class macos_network_context : public lanxc::network_context
{
  std::shared_ptr<lanxc::connection_listener_builder>
  create_connection_listener() override
  {
    return std::make_shared<macos_stream_listener_builder>();
  }

  std::shared_ptr<lanxc::connection_endpoint_builder>
  create_connection_endpoint() override
  {
    return nullptr;
  }
};
