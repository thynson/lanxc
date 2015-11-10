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

#ifndef LANXC_LINK_FORWARD_LIST_ITERATOR_HPP_INCLUDED
#define LANXC_LINK_FORWARD_LIST_ITERATOR_HPP_INCLUDED

#include "forward_list_node.hpp"

#include <iterator>


namespace lanxc
{
  namespace link
  {
    template<typename Node, typename Tag>
    class forward_list_iterator
      : public std::iterator<std::forward_iterator_tag, Node>
    {
      using node_type = forward_list_node<Node, Tag>;
    public:
      forward_list_iterator(node_type *node) noexcept
        : m_node(node)
      {}

      forward_list_iterator(const forward_list_iterator &iter) noexcept
        : m_node(iter.m_node)
      {}

      ~forward_list_iterator() = default;

      typename forward_list_iterator::reference
      operator * () const noexcept { return *internal_cast(); }

      typename forward_list_iterator::pointer
      operator -> () const noexcept { return internal_cast(); }

      forward_list_iterator &operator ++ () noexcept
      { m_node = m_node->m_next; return *this; }

      forward_list_iterator operator ++ (int) noexcept
      {
        forward_list_iterator l(m_node);
        ++(*this);
        return l;
      }

      friend bool operator ==
      (const forward_list_iterator &l, const forward_list_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator !=
      (const forward_list_iterator &l, const forward_list_iterator &r) noexcept
      { return !(l == r); }

    private:
      typename forward_list_iterator::pointer internal_cast() const noexcept
      { return static_cast<typename forward_list_iterator::pointer>(m_node); }

      node_type *m_node;
    };

    template<typename Node, typename Tag>
    class forward_list_const_iterator
      : public std::iterator<std::forward_iterator_tag, const Node>
    {
      using node_type = const forward_list_node<Node, Tag>;
    public:
      forward_list_const_iterator(node_type *node) noexcept
        : m_node(node)
      {}

      forward_list_const_iterator(const forward_list_iterator<Node, Tag> &iter)
        noexcept
        : m_node(iter.operator->())
      {}

      forward_list_const_iterator(const forward_list_const_iterator &iter) noexcept
        : m_node(iter.m_node)
      {}

      ~forward_list_const_iterator() = default;

      typename forward_list_const_iterator::reference
      operator * () const noexcept { return *internal_cast(); }

      typename forward_list_const_iterator::pointer
      operator -> () const noexcept { return internal_cast(); }

      forward_list_const_iterator &operator ++ () noexcept
      { m_node = m_node->m_next; return *this; }

      forward_list_const_iterator operator ++ (int) noexcept
      {
        forward_list_const_iterator l(m_node);
        ++(*this);
        return l;
      }

      friend bool operator == (const forward_list_const_iterator &l,
          const forward_list_const_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator != (const forward_list_const_iterator &l,
          const forward_list_const_iterator &r) noexcept
      { return !(l == r); }

    private:
      typename forward_list_const_iterator::pointer
      internal_cast() const noexcept
      {
        return
          static_cast<typename forward_list_const_iterator::pointer>(m_node);
      }

      node_type *m_node;
    };
  }
}

#endif
