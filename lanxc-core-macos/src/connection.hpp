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
#include <lanxc/core-macos/event_loop.hpp>
#include <lanxc/core-macos/event_service.hpp>
#include <lanxc/core-unix/core-posix.hpp>
#include <lanxc/core/network_context.hpp>
#include <memory>

#include <sys/socket.h>

namespace lanxc
{
  namespace macos
  {

    struct readable_event_channel : public event_channel
                                  , private virtual posix::file_descriptor
    {
      using event_channel::event_channel;

      void on_activate(std::intptr_t data, std::uint32_t flags) override;
      virtual void on_readable(std::intptr_t data, std::uint32_t flags) = 0;
    };

    struct writable_event_channel : public event_channel
                                  , private virtual posix::file_descriptor
    {
      using event_channel::event_channel;

      void on_activate(std::intptr_t data, std::uint32_t flags) override;
      virtual void on_writable(std::intptr_t data, std::uint32_t flags) = 0;
    };

    struct error_event_channel : public event_channel
                               , private virtual posix::file_descriptor
    {
      using event_channel::event_channel;

      void on_activate(std::intptr_t data, std::uint32_t flags) override;
      virtual void on_error(std::intptr_t data, std::uint32_t flags) = 0;
    };

    class macos_connection_endpoint
        : public virtual posix::file_descriptor
        , public lanxc::connection_endpoint
        , public readable_event_channel
        , public writable_event_channel
        , public error_event_channel
    {
    public:
      class builder;
      macos_connection_endpoint(event_loop &, int descriptor);

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
      builder(lanxc::macos::event_loop &el);


    };

    class macos_connection_listener
        : public virtual posix::file_descriptor
        , public lanxc::connection_listener
        , public lanxc::macos::readable_event_channel
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

        builder(lanxc::macos::event_loop &el);

        std::shared_ptr<address_builder> listen() override;

        std::shared_ptr<option_builder> set_option() override;

        std::shared_ptr<lanxc::connection_listener> build() override;

        std::shared_ptr <lanxc::connection_listener_builder>
        on(std::string address, uint16_t port) override;

        std::shared_ptr <lanxc::connection_listener_builder>
        on(uint16_t port) override;

        std::shared_ptr <lanxc::connection_listener_builder>
        on(std::string path) override;

      private:

        int create_socket_descriptor();

        lanxc::macos::event_loop &_event_loop;
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
      lanxc::macos::event_loop &_event_loop;
      lanxc::function<void(lanxc::connection_endpoint::pointer)> _callback;
      bool _stopped { false };

    };


  }
}
