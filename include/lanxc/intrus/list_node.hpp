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

#ifndef LANXC_INTRUS_LIST_NODE_HPP_INCLUDED
#define LANXC_INTRUS_LIST_NODE_HPP_INCLUDED

#include "list_config.hpp"
#include <utility>
#include <cassert>

namespace lanxc
{
  namespace intrus
  {

    template<typename Node, typename Tag>
    class list_node
    {
      using config = list_config<Tag>;
      using node_pointer = typename config::template pointer<list_node>;
    public:

      list_node() noexcept;

      ~list_node() noexcept
      {
        if (config::allow_constant_time_unlink)
          unlink_internal();
        else
          assert(is_linked() && "Node should be erased from list before destroyed");
      }

      list_node(list_node &&other) noexcept
        : list_node()
      { swap_nodes(*this, other); }

      list_node &operator = (list_node &&other) noexcept
      { swap_nodes(*this, other); }

      list_node(const list_node &) = delete;
      list_node &operator = (const list_node &) = delete;

      typename std::enable_if<config::allow_constant_time_unlink, bool>::type
      unlink() noexcept
      {
        unlink_internal();
      }

      bool is_linked() const noexcept
      { return m_next != nullptr || m_prev != nullptr; }

    private:

      bool unlink_internal() noexcept
      {
        bool ret = false;
        if (m_prev)
        {
          m_prev->m_next = m_next;
          ret = true;
        }
        if (m_next)
        {
          m_next->m_prev = m_prev;
          ret = true;
        }
        m_prev = nullptr;
        m_next = nullptr;
        return ret;
      }

      template<typename, typename> friend class list_node;
      template<typename, typename> friend class list_iterator;

      list_node (list_node *prev, list_node *next) noexcept
        : m_prev(prev), m_next(next)
      { }

      static void swap_nodes (list_node &lhs, list_node &rhs) noexcept
      {
        if (&lhs == &rhs) return;

        if (lhs.m_prev)
        { lhs.m_prev->m_next = &rhs; }

        if (lhs.m_next)
        { lhs.m_next->m_prev = &rhs; }

        if (rhs.m_prev)
        { rhs.m_prev->m_next = &lhs; }

        if (rhs.m_next)
        { rhs.m_next->m_prev = &lhs; }

        std::swap(lhs.m_prev, rhs.m_prev);
        std::swap(lhs.m_next, rhs.m_next);
      }

      node_pointer m_prev;
      node_pointer m_next;
    };

  }
}

#endif
