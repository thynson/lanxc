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

#ifndef LANXC_INTRUS_LIST_HPP_INCLUDED
#define LANXC_INTRUS_LIST_HPP_INCLUDED

#include "list_iterator.hpp"
#include "list_node.hpp"

namespace lanxc
{
  namespace intrus
  {

    template<>
    class list<void>
    {
      template<typename, typename>
      friend class list_node;

      template<typename, typename>
      friend class list_iterator;

      template<typename, typename>
      friend class list_const_iterator;

      template<typename, typename>
      friend class list;

      // Implementation details here

      template<typename List, bool>
      class enable_counter
      {
        template<typename, typename>
        friend class list;

        using size_type = std::size_t;

        void increase(size_type) noexcept {}

        void decrease(size_type) noexcept {}

        void swap_size(enable_counter &) { }

      public:
        size_type size() const
        {
          auto &l = static_cast<const List&>(*this);
          size_type s = 0;
          auto b = l.begin();
          auto e = l.end();
          while (b++ != e) s++;
          return s;
        }
      };

      template<typename List>
      class enable_counter<List, true>
      {
        template<typename, typename>
        friend class list;

        using size_type = std::size_t;

        size_type m_counter;
        enable_counter()
          : m_counter(0)
        {}

        void increase(size_type n) noexcept
        { m_counter += n; }

        void decrease(size_type n) noexcept
        { m_counter -= n; }

        void swap_size(enable_counter &n)
        { std::swap(m_counter, n.m_counter); }

      public:
        size_type size() const
        { return m_counter; }
      };


    };

    template<typename Node, typename Tag>
    class list
      : public list<void>::enable_counter<list<Node, Tag>,
          !list_config<Tag>::allow_constant_time_unlink>
    {
      using config                  = list_config<Tag>;
      using node_type               = list_node<Node, Tag>;
      using enable_counter = list<void>::enable_counter<size_t,
            !list_config<Tag>::allow_constant_time_unlink>;
    public:
      using iterator                = list_iterator<Node, Tag>;
      using const_iterator          = list_const_iterator<Node, Tag>;
      using reverse_iterator        = std::reverse_iterator<iterator>;
      using const_reverse_iterator  = std::reverse_iterator<const_iterator>;
      using value_type              = Node;
      using reference               = value_type &;
      using pointer                 = value_type *;
      using size_type               = typename enable_counter::size_type;
      using difference_type         = std::ptrdiff_t;
      using iterator_category       = std::bidirectional_iterator_tag;

      /** @brief Default constructor */
      list() noexcept
        : m_head(nullptr, &m_tail)
        , m_tail(&m_head, nullptr)
      {}

      /** @brief Destructor */
      ~list() noexcept
      { clear(); }


      /** @brief Move constructor */
      list(list &&other) noexcept
        : list()
      { swap(other); }

      /** @brief Move assign operator */
      list &operator = (list &&other) noexcept
      { swap(other); }

      list(const list &) = delete;

      list &operator = (const list &) = delete;

      bool empty() const noexcept
      { return m_head.m_next == &m_tail; }

      iterator begin() noexcept
      { return iterator(m_head.m_next); }

      reverse_iterator rbegin() noexcept
      { return reverse_iterator(end()); }

      const_iterator cbegin() const noexcept
      { return const_iterator(m_head.m_next); }

      const_iterator begin() const noexcept
      { return const_iterator(m_head.m_next); }

      const_reverse_iterator rbegin() const noexcept
      { return const_reverse_iterator(end()); }

      const_reverse_iterator crbegin() const noexcept
      { return const_reverse_iterator(end()); }

      iterator end() noexcept
      { return iterator(&m_tail); }

      reverse_iterator rend() noexcept
      { return reverse_iterator(begin()); }

      const_iterator end() const noexcept
      { return const_iterator(&m_tail); }

      const_iterator cend() const noexcept
      { return const_iterator(&m_tail); }

      const_reverse_iterator rend() const noexcept
      { return const_reverse_iterator(begin()); }

      const_reverse_iterator crend() const noexcept
      { return const_reverse_iterator(begin()); }

      /**
       * @brief Insert given value to specified position
       * @param pos The specified position
       * @param ref Reference to the element to be inserted
       */
      void insert(iterator pos, reference ref) noexcept
      {
        node_type &nref = ref;

        // Unlink first
        nref.unlink_internal();
        nref.m_prev = pos->m_prev;
        nref.m_prev->m_next = &nref;
        pos->m_prev = &nref;
        nref.m_next = &(*pos);
        this->increase(1);
      }

      /**
       * @brief Insert elements from range to specified position
       * @tparam InputIterator The type of iterators which represent the range
       * @param pos The specified position
       * @param b The begin of the range
       * @param e The end of the range
       */
      template<typename InputIterator>
        void insert(iterator pos, InputIterator b, InputIterator e)
          noexcept(noexcept(list(b, e)))
        {
          list l(b, e);
          splice(pos, l);
        }

      /**
       * @brief Erase an elements at specified position
       * @param pos The position of the elements
       */
      void erase(iterator pos) noexcept
      {
        remove(*pos);
      }

      void remove(value_type &node) noexcept
      {
        node_type &ref = node;
        ref.unlink_internal();
        this->decrease(1);
      }

      /**
       * @brief Erase all elements within specified range
       * @param b The begin of the range
       * @param e The end of the range
       */
      void erase(iterator b, iterator e) noexcept
      {
        while (b != e)
          erase(b++);
      }

      void clear()
      {
        auto ptr = m_head.m_next;
        while (ptr != &m_tail)
        {
          auto tmp = ptr->m_next;
          ptr->m_next = nullptr;
          ptr->m_prev = nullptr;
          ptr = tmp;
        }

        m_head.m_next = &m_tail;
        m_tail.m_prev = &m_head;
      }

      /**
       * @brief Transfer all elements from l to position pos in this list
       */
      void splice(iterator pos, list &l) noexcept
      {
        if (&l == this) return;

        auto b = l.begin(), e = l.end();

        this->increase(l.m_counter.m_counter);
        l.decrease(l.m_coutner.m_counter);

        node_type &x = *(b->m_prev), &y = *(e->m_prev);
        x.m_next = &(*e);
        e->m_prev = &x;

        b->m_prev = pos->m_prev;
        y.m_next = &(*pos);
        b->m_prev->m_next = &(*b);
        y.m_next->m_prev = &y;
      }

      /**
       * @brief Transfer all elements from l to position pos in this list
       */
      void splice(iterator pos, list &&l) noexcept
      {
        if (&l == this) return;
        splice(pos, l.begin(), l.end());
      }

      /**
       * @brief Transfer all elements ranged from b to e to position pos
       * in this list
       */
      void splice(iterator pos, list &l, iterator b, iterator e) noexcept
      {
        if (b == e) return;

        size_t s = 0;
        for (auto i = b; i != e; ++i) s++;

        this->increase(s);
        l.decrease(s);

        node_type &x = *(b->m_prev), &y = *(e->m_prev);
        x.m_next = &(*e);
        e->m_prev = &x;

        b->m_prev = pos->m_prev;
        y.m_next = &(*pos);
        b->m_prev->m_next = &(*b);
        y.m_next->m_prev = &y;
      }

      void swap(list &&l) noexcept
      { return swap(l); }

      void swap(list &l) noexcept
      {
        list *lhs = this, *rhs = &l;

        if (lhs == rhs) return;

        if (lhs->empty())
        {
          if (rhs->empty())
            return;
          else
            std::swap(lhs, rhs);
        }

        if (rhs->empty())
        {
          lhs->m_head.m_next->m_prev = &rhs->m_head;
          lhs->m_tail.m_prev->m_next = &rhs->m_tail;
          rhs->m_head.m_next = lhs->m_head.m_next;
          rhs->m_tail.m_prev = lhs->m_tail.m_prev;
          lhs->m_head.m_next = &lhs->m_tail;
          lhs->m_tail.m_prev = &lhs->m_head;
        }
        else
        {
          std::swap(lhs->m_head.m_next->m_prev,
                    rhs->m_head.m_next->m_prev);
          std::swap(lhs->m_tail.m_prev->m_next,
                    rhs->m_tail.m_prev->m_next);
          std::swap(lhs->m_head.m_next, rhs->m_head.m_next);
          std::swap(lhs->m_tail.m_prev, rhs->m_tail.m_prev);
        }
        lhs->swap_size(*rhs);
      }

    private:

      node_type m_head;
      node_type m_tail;

    };

  }
}
#endif
