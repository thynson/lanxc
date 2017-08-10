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

#include <lanxc/core/future.hpp>
#include <lanxc/config.hpp>

#include <cstdint>

namespace lanxc
{

  class io_exception
  {

  };

  /**
   * @brief The content of an readable stream has been discarded explicitly
   */
  class stream_discarded_exception : public io_exception
  {

  };

  /**
   * @brief An writable stream has already closed
   */
  class stream_closed_exception : public io_exception
  {

  };

  class writable_stream;

  class LANXC_CORE_EXPORT buffer_manager
  {
  public:
    virtual ~buffer_manager() = 0;
    virtual std::uint8_t *acquire(std::size_t size) = 0;
    virtual void release(std::uint8_t *data, std::size_t size) noexcept = 0;

  };

  class readable_buffer
  {
    friend class buffer_manager;
  public:
    ~readable_buffer()
    {
      _bm.release(_data, _size);
    }


    readable_buffer(readable_buffer &&other) noexcept
      : _bm(other._bm)
      , _data{}
      , _size{}
    {
      std::swap(_data, other._data);
      std::swap(_size, other._size);
    }

    readable_buffer &operator = (readable_buffer &&other) noexcept
    {
      this->~readable_buffer();
      new (this) readable_buffer(std::move(other));
      return *this;
    }

  private:
    buffer_manager &_bm;
    std::uint8_t *_data;
    std::size_t _size;


  };

  class writable_buffer
  {
    friend class buffer_manager;
  public:
    ~writable_buffer()
    {
      _bm.release(_data, _size);
    }


    writable_buffer(writable_buffer &&other) noexcept
      : _bm(other._bm)
      , _data{}
      , _size{}
    {
      std::swap(_data, other._data);
      std::swap(_size, other._size);
    }

    writable_buffer &operator = (writable_buffer &&other) noexcept
    {
      this->~writable_buffer();
      new (this) writable_buffer(std::move(other));
      return *this;
    }

  private:
    buffer_manager &_bm;
    std::uint8_t *_data;
    std::size_t _size;
  };

  class buffer_factory
  {

  };

  class readable_stream
  {
  public:
    virtual future<size_t, readable_buffer>
    read(std::size_t size, std::size_t watermark) = 0;

    virtual void discard() = 0;

  };


  class writable_stream
  {
  public:

    virtual writable_buffer allocate_buffer(std::size_t size) = 0;

    virtual std::size_t write(writable_buffer b) = 0;

    virtual void close() = 0;

    virtual future<> flush() = 0;

  };



}
