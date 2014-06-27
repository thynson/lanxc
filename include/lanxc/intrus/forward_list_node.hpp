/*
 * Copyright (C) 2014 LAN Xingcan
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

#ifndef LANXC_INTRUS_FORWARD_LIST_NODE_HPP_INCLUDED
#define LANXC_INTRUS_FORWARD_LIST_NODE_HPP_INCLUDED

#include "forward_list_config.hpp"

#include <cassert>

namespace lanxc
{
  namespace intrus
  {

    template<typename Node, typename Tag>
    class forward_list_node
    {

      using config = forward_list_config<Tag>;
      using node_pointer = typename config::template pointer<forward_list_node>;

      template<typename, typename> friend class forward_list;
      template<typename, typename> friend class forward_list_iterator;
      template<typename, typename> friend class forward_list_const_iterator;

    public:

      forward_list_node() noexcept
        : m_next(nullptr)
      {}

      ~forward_list_node() noexcept
      { assert (m_next == nullptr); }

      forward_list_node(const forward_list_node&) = delete;
      forward_list_node &operator = (const forward_list_node&) = delete;
      forward_list_node(forward_list_node&&) = delete;
      forward_list_node &operator = (forward_list_node&&) = delete;

      bool is_linked() const noexcept
      { return m_next != nullptr; }

    private:
      node_pointer m_next;
    };


  }
}
#endif
