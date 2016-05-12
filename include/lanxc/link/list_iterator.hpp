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

#ifndef LANXC_LINK_LIST_ITERATOR_HPP_INCLUDED
#define LANXC_LINK_LIST_ITERATOR_HPP_INCLUDED

#include "list_node.hpp"

#include <iterator>

namespace lanxc
{
  namespace link
  {

    /**
     * @brief List iterator
     * @ingroup intrusive_list
     */
    template<typename Node, typename Tag>
    class list_iterator
      : public std::iterator<std::bidirectional_iterator_tag, Node,
          std::ptrdiff_t, Node *, Node &>
    {
      using node_type = list_node<Node, Tag>;

    public:

      explicit list_iterator (node_type *p) noexcept
        : m_node(p)
      {}

      list_iterator(const list_iterator &iter) noexcept
        : m_node(iter.m_node)
      {}

      typename list_iterator::reference
      operator * () const noexcept { return *internal_cast(); }

      typename list_iterator::pointer
      operator -> () const noexcept { return internal_cast(); }

      list_iterator &operator ++ () noexcept
      { m_node = m_node->m_next; return *this; }

      list_iterator &operator -- () noexcept
      { m_node = m_node->m_prev; return *this; }

      list_iterator operator ++ (int) noexcept
      {
        list_iterator l(m_node);
        ++(*this);
        return l;
      }

      list_iterator operator -- (int) noexcept
      {
        list_iterator l(m_node);
        --(*this);
        return l;
      }

      friend bool operator ==
      (const list_iterator &l, const list_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator !=
      (const list_iterator &l, const list_iterator &r) noexcept
      { return !(l == r); }

    private:
      typename list_iterator::pointer internal_cast() const noexcept
      { return static_cast<typename list_iterator::pointer>(m_node); }

      node_type *m_node;
    };

    template<typename Node, typename Tag>
    class list_const_iterator
      : public std::iterator<std::bidirectional_iterator_tag, const Node,
          std::ptrdiff_t, const Node *, const Node &>
    {
      using node_type = const list_node<Node, Tag>;
    public:
      explicit list_const_iterator(node_type *p) noexcept
        : m_node(p)
      {}

      list_const_iterator(list_iterator<Node, Tag> &iter) noexcept
        : list_const_iterator(iter.operator->())
      {}

      list_const_iterator(const list_const_iterator &i) noexcept
        : m_node(i.m_node)
      {}

      ~list_const_iterator() = default;

      typename list_const_iterator::reference
      operator * () const noexcept { return *internal_cast(); }

      typename list_const_iterator::pointer
      operator -> () const noexcept { return internal_cast(); }

      list_const_iterator &operator ++ () noexcept
      { m_node = m_node->m_next; return *this; }

      list_const_iterator &operator -- () noexcept
      { m_node = m_node->m_prev; return *this; }

      list_const_iterator operator ++ (int) noexcept
      {
        list_const_iterator l(m_node);
        ++(*this);
        return l;
      }

      list_const_iterator operator -- (int) noexcept
      {
        list_const_iterator l(m_node);
        --(*this);
        return l;
      }

      friend bool operator ==
      (const list_const_iterator &l, const list_const_iterator &r) noexcept
      { return  l.m_node == r.m_node; }

      friend bool operator !=
      (const list_const_iterator &l, const list_const_iterator &r) noexcept
      { return !(l == r); }
    private:

      typename list_const_iterator::pointer internal_cast() const noexcept
      { return static_cast<typename list_const_iterator::pointer>(m_node); }

      node_type *m_node;
    };

  }
}

#endif
