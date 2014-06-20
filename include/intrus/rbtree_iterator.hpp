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

#ifndef LANXC_INTRUS_RBTREE_ITERATOR_HPP_INLCUDED
#define LANXC_INTRUS_RBTREE_ITERATOR_HPP_INLCUDED

#include "rbtree_node.hpp"

#include <iterator>

namespace lanxc
{
  namespace intrus
  {

    /**
     * @brief Iterator for rbtrree
     * @ingroup intrusive_rbtree
     */
    template<typename Index, typename Node, typename Tag>
    class rbtree_iterator
      : public std::iterator<std::bidirectional_iterator_tag,
                             Node, std::ptrdiff_t, Node *, Node &>
    {
      using node_type = rbtree_node<Index, Node, Tag,
                                    rbtree_node<void, void>>;
    public:

      explicit rbtree_iterator(node_type *x) noexcept
        : m_node(x)
      {  }

      rbtree_iterator(const rbtree_iterator &i) noexcept
        : m_node(i.m_node)
      {  }

      rbtree_iterator &operator = (const rbtree_iterator &i)
      { m_node = i.m_node; return *this; }

      ~rbtree_iterator() noexcept = default;

      operator bool () const noexcept
      { return !m_node->is_container_node; }

      typename rbtree_iterator::reference
      operator * () const noexcept { return *internal_cast(); }

      typename rbtree_iterator::pointer
      operator -> () const noexcept { return internal_cast(); }

      rbtree_iterator &operator ++ () noexcept
      {
        m_node = m_node->next();
        return *this;
      }

      rbtree_iterator operator ++ (int) noexcept
      {
        auto ret(*this);
        ++(*this);
        return ret;
      }

      rbtree_iterator &operator -- () noexcept
      {
        m_node = m_node->prev();
        return *this;
      }

      rbtree_iterator operator -- (int) noexcept
      {
        auto ret(*this);
        --(*this);
        return ret;
      }

      friend bool operator == (const rbtree_iterator &l,
          const rbtree_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator != (const rbtree_iterator &l,
          const rbtree_iterator &r) noexcept
      { return !(l.m_node == r.m_node); }

    private:

      typename rbtree_iterator::pointer internal_cast() const noexcept
      { return static_cast<typename rbtree_iterator::pointer>(m_node); }

      node_type *m_node;

    };


    /**
     * @brief Constant iterator for rbtrree
     * @ingroup intrusive_rbtree
     */
    template<typename Index, typename Node, typename Tag>
    class rbtree_const_iterator
      : public std::iterator<std::bidirectional_iterator_tag, Node>
    {
      using node_type = const rbtree_node<Index, Node, Tag>;
      using rbtree_iterator = ::lanxc::intrus::rbtree_iterator<Index, Node, Tag>;
    public:
      explicit rbtree_const_iterator(node_type *x) noexcept
        : m_node(x)
      { }

      rbtree_const_iterator(const rbtree_iterator &iter) noexcept
        : rbtree_const_iterator(iter.operator->())
      { }

      operator bool () const noexcept
      { return !m_node->is_container_node; }

      typename rbtree_const_iterator::reference
      operator * () const noexcept { return *internal_cast(); }

      typename rbtree_const_iterator::pointer
      operator -> () const noexcept { return internal_cast(); }

      rbtree_const_iterator &operator ++ () noexcept
      {
        m_node = m_node->next();
        return *this;
      }

      rbtree_const_iterator operator ++ (int) noexcept
      {
        auto ret(*this);
        ++(*this);
        return ret;
      }

      rbtree_const_iterator &operator -- () noexcept
      {
        m_node = m_node->prev();
        return *this;
      }

      rbtree_const_iterator operator -- (int) noexcept
      {
        auto ret(*this);
        --(*this);
        return ret;
      }

      friend bool operator == (const rbtree_const_iterator &l,
          const rbtree_const_iterator &r) noexcept
      { return l.m_node == r.m_node; }

      friend bool operator != (const rbtree_const_iterator &l,
          const rbtree_const_iterator &r) noexcept
      { return !(l.m_node == r.m_node); }
    private:

      typename rbtree_const_iterator::pointer internal_cast() const noexcept
      { return static_cast<typename rbtree_const_iterator::pointer>(m_node); }

      node_type *m_node;
    };

  }
}
#endif
