/*
 * Copyright (C) 2015 LAN Xingcan
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
#include "list_config.hpp"
#include <utility>
#include <cassert>

namespace lanxc
{
  namespace link
  {

    template<>
    class list_node<void, void>
    {
      template<typename, typename>
      friend class list_node;

    public:

      template<typename Node, typename Tag,
          typename = typename std::conditional<
              list_config<Tag>::allow_constant_time_unlink,
                  std::true_type, std::false_type>::type>
      class enable_unlink
      {
      public:
        using size_type = std::size_t;
      };

      template<typename Node, typename Tag>
      class enable_unlink<Node, Tag, typename std::enable_if<
          !list_config<Tag>::allow_constant_time_unlink, std::false_type>::type>
      {
      protected:
        ~enable_unlink()
        { assert(!(static_cast<list_node<Node, Tag>*>(this)->is_linked())); }
      };

      template<typename Node, typename Tag>
      class enable_unlink<Node, Tag, typename std::enable_if<
          list_config<Tag>::allow_constant_time_unlink, std::true_type>::type>
      {
      public:
        void unlink()
        { static_cast<Node*>(this)->list_node<Node, Tag>::unlink_internal(); }
      protected:
        ~enable_unlink()
        { this->enable_unlink::unlink(); }
      };

    };

    /**
     * @brief List node
     * @ingroup intrusive_list
     */
    template<typename Node, typename Tag>
    class list_node : public list_node<void, void>::enable_unlink<Node, Tag>
    {
      using config = list_config<Tag>;
      using node_pointer = typename config::template pointer<list_node>;
      using const_node_pointer = typename config::template pointer<list_node>;
      friend class list<Node, Tag>;
      template<typename, typename> friend class list_node;
      template<typename, typename> friend class list_iterator;
      template<typename, typename> friend class list_const_iterator;

    public:

      list_node() noexcept
        : m_prev(nullptr), m_next(nullptr)
      { }

      ~list_node() noexcept
      { }

      list_node(list_node &&other) noexcept
        : list_node()
      { swap_nodes(*this, other); }

      list_node &operator = (list_node &&other) noexcept
      {
        swap_nodes(*this, other);
        return *this;
      }

      list_node(const list_node &) = delete;
      list_node &operator = (const list_node &) = delete;

      bool is_linked() const noexcept
      { return m_next != nullptr && m_prev != nullptr; }

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

