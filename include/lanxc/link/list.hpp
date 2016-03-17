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

#ifndef LANXC_LINK_LIST_HPP_INCLUDED
#define LANXC_LINK_LIST_HPP_INCLUDED

#include "list_iterator.hpp"
#include "list_node.hpp"
#include "../functional.hpp"

#include <algorithm>

namespace lanxc
{
  namespace link
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

      template<typename Node, typename Tag,
        typename = typename std::conditional<
            list_config<Tag>::allow_constant_time_unlink,
            std::true_type, std::false_type>::type>
      class enable_counter;

      template<typename Node, typename Tag>
      class enable_counter<Node, Tag,
          typename std::enable_if<
              !list_config<Tag>::allow_constant_time_unlink, std::false_type>::type>
      {
        template<typename, typename>
        friend class list;

        using size_type = std::size_t;

        size_type m_counter;
        enable_counter()
          : m_counter(0)
        { }

        void increase() noexcept
        { m_counter += 1; }

        void decrease() noexcept
        { m_counter -= 1; }

        void transfer(enable_counter &n) noexcept
        {
          m_counter += n.m_counter;
          n.m_counter = 0;
        }

        void swap_size(enable_counter &n) noexcept
        { std::swap(m_counter, n.m_counter); }

      public:
        size_type get_size() const
        { return m_counter; }
      };


      template<typename Node, typename Tag>
      class enable_counter<Node, Tag,
          typename std::enable_if<
              list_config<Tag>::allow_constant_time_unlink, std::true_type>::type>

      {
        template<typename, typename>
        friend class list;

        using size_type = std::size_t;

        void increase() noexcept {}

        void decrease() noexcept {}

        void swap_size(enable_counter &) noexcept { }

        void transfer(enable_counter &) noexcept { }

      public:
        size_type get_size() const
        {
          const list<Node, Tag> &l = static_cast<const list<Node, Tag>&>(*this);
          size_type s = 0;
          auto b = l.cbegin();
          auto e = l.cend();
          while (b++ != e) s++;
          return s;
        }
      };

    };

    /**
     * @brief Intrusive bidirectional linked list
     * @ingroup intrusive_list
     */
    template<typename Node, typename Tag>
    class list : private list<void, void>::enable_counter<Node, Tag>
    {
      using config                  = list_config<Tag>;
      using node_type               = list_node<Node, Tag>;
      using enable_counter          = list<void, void>::enable_counter<Node, Tag>;
    public:
      using iterator                = list_iterator<Node, Tag>;
      using const_iterator          = list_const_iterator<Node, Tag>;
      using reverse_iterator        = std::reverse_iterator<iterator>;
      using const_reverse_iterator  = std::reverse_iterator<const_iterator>;
      using value_type              = Node;
      using reference               = value_type &;
      using pointer                 = value_type *;
      using const_reference         = const value_type &;
      using const_pointer           = const value_type *;
      using size_type               = typename enable_counter::size_type;
      using difference_type         = std::ptrdiff_t;
      using iterator_category       = std::bidirectional_iterator_tag;

      /** @brief Default constructor */
      list() noexcept
        : m_head(nullptr, &m_tail)
        , m_tail(&m_head, nullptr)
      {}

      template<typename InputIterator>
      list(InputIterator b, InputIterator e)
        : list()
      {
        while (b != e)
        {
          this->insert(end(), b);
          ++b;
        }
      }

      /** @brief Destructor */
      ~list() noexcept
      { clear(); }


      /** @brief Move constructor */
      list(list &&other) noexcept
        : list()
      { swap(other); }

      /** @brief Move assign operator */
      list &operator = (list &&other) noexcept
      {
        swap(other);
        return *this;
      }

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

      reference front() { return *begin(); }

      const_reference front() const { return *begin(); }

      reference back() { return *rbegin(); }

      const_reference back() const { return *rbegin(); }

      size_type size() const noexcept
      { return enable_counter::get_size(); }

      /**
       * @brief Insert given value to specified position
       * @param pos The specified position
       * @param ref Reference to the element to be inserted
       */
      void insert(iterator pos, reference ref) noexcept
      {
        node_type &node_ref = ref;

        // Unlink first
        node_ref.unlink_internal();
        node_ref.m_prev = pos->m_prev;
        node_ref.m_prev->m_next = &node_ref;
        pos->m_prev = &node_ref;
        node_ref.m_next = &(*pos);
        this->increase();
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
        node_type &ref = *pos;
        if (pos->is_linked())
        {
          ref.unlink_internal();
          this->decrease();
        }
      }


      /**
       * @brief Insert a elements to the front of this list
       * @param ref The reference to the element to be inserted
       */
      void push_front(reference ref) noexcept
      { insert(begin(), ref); }

      /**
       * @brief Insert a elements to the back of this list
       * @param ref The reference to the element to be inserted
       */
      void push_back(reference ref) noexcept
      { insert(end(), ref); }

      /**
       * @brief Remove the first element from  the list
       */
      void pop_front() noexcept
      { erase(begin()); }

      /**
       * @brief Remove the last element from the list
       */
      void pop_back() noexcept
      { erase(iterator(m_tail.m_prev)); }

      /**
       * @brief Remove a node from this list
       *
       * @note Unlink std::list::remove, this function doest not requires
       * operator = for value_type be implemented. So this function removes
       * exactly the node that passed as argument from the list.
       * @see remove_if
       */
      template<typename BinaryPredicate=equals_to<Node>>
      void remove(const_reference val,
          BinaryPredicate &&predicate = BinaryPredicate()) noexcept
      {
        auto b = begin(), e = end();
        while (b != e)
        {
          auto c = b++;
          if (predicate(val, *c))
            erase(c);
        }
      }

      /**
       * @brief Remove nodes from element which was filtered by predicate
       * @param predicate Functor to judge whether a node should be removed
       */
      template<typename Predicate>
      void remove_if(Predicate &&predicate)
        noexcept(noexcept(predicate(std::declval<Node&>())))
      {
        auto k = begin();
        auto e = end();
        while (k != e)
        {
          auto c = k++;
          if (predicate(*c))
            erase(c);
        }
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

      /**
       * @brief remove all nodes from this list
       */
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
        if (l.empty()) return;

        auto b = l.begin(), e = l.end();

        this->transfer(l);

        node_type &x = *(b->m_prev), &y = *(e->m_prev);
        x.m_next = &(*e);
        e->m_prev = &x;

        b->m_prev = pos->m_prev;
        y.m_next = &(*pos);
        b->m_prev->m_next = &(*b);
        y.m_next->m_prev = &y;
      }

      /**
       * @brief Transfer all elements ranged from b to e to position pos
       * in this list
       */
      void splice(iterator pos, list &l, iterator b, iterator e) noexcept
      {

        if (&l == this) return;
        if (b == e) return;

        for (auto i = b; i != e; ++i) {

          this->increase();
          l.decrease();
        }

        node_type &x = *(b->m_prev), &y = *(e->m_prev);
        x.m_next = &(*e);
        e->m_prev = &x;

        b->m_prev = pos->m_prev;
        y.m_next = &(*pos);
        b->m_prev->m_next = &(*b);
        y.m_next->m_prev = &y;
      }

      /** @brief Swap content with another list */
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

      void reverse()
      {
        list tmp;
        while(!empty())
        {
          auto &n = front();
          pop_front();
          tmp.push_front(n);
        }
        swap(tmp);
      }

      /**
       * @brief Merge a sorted part of from b to e of another list with
       * specified strict comparator
       * @param l The list which part of it will be merged @param b Begin of
       * element to be merged
       * @param e End of element to be merged
       * @param comp The specified comparator
       * @note both of this list and the part to be merged should be
       * sorted with specified comparator, and b and e should be iterators of
       * distinct list
       */
      template<typename Comparator=less<Node>>
      void merge(list &l, iterator b, iterator e,
          Comparator &&comp = Comparator()) noexcept(noexcept(comp(*b, *e)))
      {
        auto p = begin(), q = end();

        while (p != q && b != e)
        {
          if (std::forward<Comparator>(comp)(*b, *p))
          {
            auto &n = *b++;
            l.erase(iterator(&n));
            insert(p, n);
          }
          else
            ++p;
        }

        if (b != e)
          splice(q, l, b, e);
      }

      /**
       * @brief Merge another sorted list with comparator @p c
       * @param l The list to be merged
       * @param comp The specified strict weak ordering comparator
       * @note both of this list and the part to be merged should be sorted
       * with specified comparator, and b and e should be iterators of
       * distinct list
       */
      template<typename Comparator=less<Node>>
      void merge(list &l, Comparator &&c = Comparator())
        noexcept(noexcept(std::forward<Comparator>(c)(*l.begin(), *l.end())))
      { merge(l, l.begin(), l.end(), std::forward<Comparator>(c)); }

      /**
       * @brief Erase duplicate elements with specified binary predicate
       * @param predicate The specified binary predicate
       */
      template<typename BinaryPredicate=equals_to<Node>>
      void unique(BinaryPredicate && predicate = BinaryPredicate())
        noexcept(noexcept(std::declval<list>().unique
              (std::declval<iterator>(), std::declval<iterator>(),
               std::forward<BinaryPredicate>(predicate))))
      {
        unique(begin(), end(),
            std::forward<BinaryPredicate>(predicate));
      }


      /**
       * @brief Sort this list with specified comparator
       * @param comp The comparator
       */
      template<typename Comparator=less<Node>>
      void sort(Comparator &&comp = Comparator())
        noexcept(noexcept(comp(std::declval<Node>(), std::declval<Node>())))
      {

        if (empty() || ++begin() == end()) return;

        list carry;
        list tmp[64];
        list *fill = &tmp[0];
        list *counter;

        do
        {
          auto &n = *begin();
          pop_front();
          carry.insert(carry.begin(), n);

          for (counter = &tmp[0];
              counter != fill && !counter->empty();
              ++counter)
          {
            counter->merge(carry, std::forward<Comparator>(comp));
            counter->swap(carry);
          }

          carry.swap(*counter);
          if (counter == fill)
            ++fill;
        }
        while(!empty());

        for (counter = &tmp[1]; counter != fill; ++counter)
          counter->merge(*(counter-1), std::forward<Comparator>(comp));
        swap(*(fill - 1));
      }


      /**
       * @brief Erase duplicate elements with specified binary predicate in
       * range [b, e)
       * @param b Begin of the range
       * @param e End of the range
       * @param predicate The specified binary predicate
       */
      template<typename BinaryPredicate>
      void unique(iterator b, iterator e,
          BinaryPredicate &&predicate = BinaryPredicate())
        noexcept(noexcept(predicate(*b, *e)))
      {
        if (b == e) return;
        auto n = b;
        while(++n != b)
        {
          if (predicate(*b, *n))
            erase(n);
          else
            b = n;
          n = b;
        }
      }

    private:
      node_type m_head;
      node_type m_tail;

    };

    template<typename Node, typename Tag>
      inline bool operator == (const list<Node, Tag> &x,
          const list<Node, Tag> &y) noexcept
      {
        auto i = x.cbegin(), j = y.cbegin();
        auto m = x.cend(), n = y.cend();

        while (i != m && j != n && *i == *j)
        { ++i; ++j; }

        return i == m && j == n;
      }

    template<typename Node, typename Tag>
      inline bool operator != (const list<Node, Tag> &x,
          const list<Node, Tag> &y) noexcept
      { return !(x == y); }

    template<typename Node, typename Tag>
      inline bool operator < (const list<Node, Tag> &x,
          const list<Node, Tag> &y) noexcept
      {
        if (&x == &y) return false;
        return std::lexicographical_compare(x.begin(), x.end(),
            y.begin(), y.end());
      }

    template<typename Node, typename Tag>
      inline bool operator <= (const list<Node, Tag> &x,
          const list<Node, Tag> &y) noexcept
      { return !(y < x); }

    template<typename Node, typename Tag>
      inline bool operator > (const list<Node, Tag> &x,
          const list<Node, Tag> &y) noexcept
      { return y < x; }

    template<typename Node, typename Tag>
      inline bool operator >= (const list<Node, Tag> &x,
          const list<Node, Tag> &y) noexcept
      { return !(y > x); }

  }
}
#endif
