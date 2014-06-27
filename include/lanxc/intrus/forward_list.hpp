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

#ifndef LANXC_INTRUS_FORWARD_LIST_HPP_INCLUDED
#define LANXC_INTRUS_FORWARD_LIST_HPP_INCLUDED

#include "forward_list_iterator.hpp"

#include "../functional.hpp"

namespace lanxc
{
  namespace intrus
  {
    template<typename Node, typename Tag>
    class forward_list
    {
      using config            = forward_list_config<Tag>;
      using node_type         = forward_list_node<Node, Tag>;
    public:

      using iterator          = forward_list_iterator<Node, Tag>;
      using const_iterator    = forward_list_const_iterator<Node, Tag>;
      using value_type        = Node;
      using reference         = value_type &;
      using pointer           = value_type *;
      using const_reference   = const value_type &;
      using const_pointer     = const value_type *;
      using size_type         = std::size_t;
      using difference_type   = std::ptrdiff_t;
      using iterator_category = std::forward_iterator_tag;

      forward_list() noexcept = default;

      ~forward_list()
      { clear(); }

      iterator before_begin() noexcept
      { return iterator (&m_head); }

      const_iterator before_begin() const noexcept
      { return const_iterator (&m_head); }

      const_iterator cbefore_begin() const noexcept
      { return const_iterator (&m_head); }

      iterator begin() noexcept
      { return iterator(m_head->m_next); }

      const_iterator begin() const noexcept
      { return iterator(m_head->m_next); }

      const_iterator cbegin() const noexcept
      { return iterator(m_head->m_next); }

      iterator end() noexcept
      { return iterator(nullptr); }

      const_iterator end() const noexcept
      { return const_iterator(nullptr); }

      const_iterator cend() const noexcept
      { return const_iterator(nullptr); }

      reference front() noexcept
      { return *begin(); }

      const reference front() const noexcept
      { return *begin(); }

      bool empty() const noexcept
      { return m_head.is_linked(); }

      size_type size() const noexcept
      { return m_size; }

      void clear() noexcept
      {
        for (auto *p = m_head; p != nullptr; )
        {
          auto *c = p;
          p = p->m_next;
          c->m_next = nullptr;
        }
        m_tail->m_next = nullptr;
      }

      iterator insert_after(iterator pos, reference ref) noexcept
      {
        node_type &n = ref;
        node_type &p = *pos;
        n.m_next = p->m_next;
        if (p->m_next == nullptr)
          m_tail->m_next = &n;
        p->m_next = &n;
        return iterator(&n);
      }

      iterator erase_after(iterator pos) noexcept
      {
        node_type &n = *pos;
        n.m_next = n.m_next->m_next;
        if (n.m_next == nullptr)
          m_tail->m_next = &n;
        return iterator(n.m_next);
      }

      void push_front(reference ref) noexcept
      { insert_after(before_begin(), ref); }

      void pop_front() noexcept
      { erase_after(before_begin()); }

      void swap(forward_list &l) noexcept
      {
        std::swap(l.m_head.m_next, m_head.m_next);
        std::swap(l.m_tail.m_next, m_tail.m_next);
        std::swap(l.m_size, m_size);
      }

      void swap(forward_list &&l) noexcept
      { swap(l); }

      void splice_after(iterator pos, forward_list &l)
      {
        auto next = pos->forward_list_node<Node, Tag>::m_next;
        l.m_tail.m_next = next;
        if (next == nullptr)
          m_tail->m_next = l.m_tail.m_next;
        pos->forward_list_node<Node, Tag>::m_next = l.m_head.m_next;
        l.m_head.m_next = nullptr;
        l.m_tail.m_next = nullptr;
        m_size += l.m_size;
        l.m_size = 0;
      }

      void splice_after(iterator pos, forward_list &&l)
      { splice_after(pos, l); }

      void splice_after(iterator pos, forward_list &l, iterator b);
      void splice_after(iterator pos, forward_list &&l, iterator b)
      { splice_after(pos, l, b); }

      void splice_after(iterator pos, forward_list &l, iterator b, iterator e);
      void splice_after(iterator pos, forward_list &&l, iterator b, iterator e)
      { splice_after(pos, l, b, e); }

      template<typename Comparator = less<Node>>
      void merge(forward_list &l, Comparator &&comp = Comparator());

      template<typename Comparator = less<Node>>
      void merge(forward_list &l, iterator b, iterator e,
          Comparator &&comp = Comparator());

      template<typename BinaryPredicate>
      void remove(const_reference &val, BinaryPredicate &&binpred)
      {
        auto i = before_begin();
        for ( ; ; )
        {
          auto n = iterator(i->forward_list_node<Node, Tag>::m_next);
          if (!n)
            return;
          if (binpred(val, *n))
            erase_after(i);
          else
            i = n;
        }
      }

      template<typename Predicate>
      void remove_if(Predicate &&pred)
      {
        auto i = before_begin();
        for ( ; ; )
        {
          auto n = iteraotr(i->forward_list_node<Node, Tag>::m_next);
          if (!n)
            return;
          if (pred(*n))
            erase_after(i);
          else
            i = n;
        }
      }

      void reverse() noexcept
      {
        forward_list l;
        while(!empty())
        {
          auto &ref = front();
          pop_front();
          l.push_front(ref);
        }
        swap(l);
      }

      template<typename BinaryPredicate>
      void unique(BinaryPredicate &&binpred);

      template<typename BinaryPredicate>
      void unique(iterator b, BinaryPredicate &&binpred);

      template<typename BinaryPredicate>
      void unique(iterator b, iterator e, BinaryPredicate &&binpred);

    private:
      node_type m_head;
      node_type m_tail;
      size_type m_size;
    };

  }
}
#endif
