/*
 * Copyright (C) 2015 - 2016 LAN Xingcan
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
#include "rbtree_config.hpp"
#include "rbtree_node.hpp"
#include "rbtree_iterator.hpp"

#include <iterator>
#include <type_traits>
#include <algorithm>

namespace lanxc
{
  namespace link
  {

    /**
     * @brief RBTree container
     * @ingroup intrusive_rbtree
     */
    template<typename Index, typename Node, typename Tag>
    class rbtree
    {
      using detail                  = rbtree_node<void, void>;
      using config                  = rbtree_config<Tag>;
      using default_insert_policy   = typename config::default_insert_policy;
      using default_lookup_policy   = typename config::default_lookup_policy;

      using node_type
          = rbtree_node<void, void>::node<Index, Node, Tag>;
      /**
       * @brief SFINAE check for lookup policy
       * @tparam Policy Type of lookup policy
       * @tparam Result SFINAE Result
       */
      template<typename Policy, typename Result = void>
      using lookup_policy_sfinae
          = typename detail::lookup_policy_sfinae<Policy, Result>;

      /**
       * @brief SFINAE check for insert policy
       * @tparam Policy Type of insert policy
       * @tparam Result SFINAE Result
       */
      template<typename Policy, typename Result = void>
      using insert_policy_sfinae
          = typename detail::insert_policy_sfinae<Policy, Result>;

    public:

      using iterator               = rbtree_iterator<Index, Node, Tag>;
      using const_iterator         = rbtree_const_iterator<Index, Node, Tag>;
      using reverse_iterator       = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      using value_type             = Node;
      using reference              = value_type &;
      using pointer                = value_type *;
      using const_reference        = const value_type &;
      using const_pointer          = const value_type *;
      using size_type              = std::size_t;
      using difference_type        = std::ptrdiff_t;

      rbtree() noexcept
        : m_container_node()
      { }

      ~rbtree() noexcept
      { clear(); };

      rbtree(rbtree &&t) noexcept
        : m_container_node(std::move(t.m_container_node))
      {  }

      rbtree &operator = (rbtree &&t) noexcept
      {
        if (&t != this)
        {
          this->~rbtree();
          new (this) rbtree(std::move(t));
        }
        return *this;
      }

      /** @brief Tests if this rbtree contains no element */
      bool empty() const noexcept
      { return m_container_node.is_empty_container_node(); }

      /** @brief Counts the elements in this tree */
      size_type size() const noexcept
      {
        return m_container_node.m_size;
      }

      /** @brief Get an iterator point to the first element */
      iterator begin() noexcept
      { return iterator(m_container_node.front_of_container()); }

      /** @brief Get a const iterator point to the first element */
      const_iterator begin() const noexcept
      { return const_iterator(m_container_node.front_of_container()); }

      /** @brief Get a const iterator point to the first element */
      const_iterator cbegin() const noexcept
      { return const_iterator(m_container_node.front_of_container()); }

      /** @brief Get a reverse iterator point to the last element */
      reverse_iterator rbegin() noexcept
      { return reverse_iterator(end()); }

      /** @brief Get a const reverse iterator point to the last element */
      const_reverse_iterator rbegin() const noexcept
      { return const_reverse_iterator(end()); }

      /** @brief Get a const reverse iterator point to the last element */
      const_reverse_iterator crbegin() const noexcept
      { return const_reverse_iterator(end()); }

      /**@brief Get an iterator point to the end of this tree */
      iterator end() noexcept
      { return iterator(&m_container_node); }

      /**@brief Get a const iterator point to the end of this tree */
      const_iterator end() const noexcept
      { return const_iterator(&m_container_node); }

      /**@brief Get a const iterator point to the end of this tree */
      const_iterator cend() const noexcept
      { return const_iterator(&m_container_node); }

      /**@brief Get an iterator point to the reverse end of this tree */
      reverse_iterator rend() noexcept
      { return reverse_iterator(begin()); }

      /**@brief Get a const iterator point to the reverse end of this tree */
      const_reverse_iterator rend() const noexcept
      { return const_reverse_iterator(begin()); }

      /**@brief Get a const iterator point to the reverse end of this tree */
      const_reverse_iterator crend() const noexcept
      { return const_reverse_iterator(begin()); }

      /**
       * @brief Get a reference to the first element
       * @note User should ensure this tree is not empty
       */
      reference front() noexcept
      { return *begin(); }

      /**
       * @brief Get a const reference to the first element
       * @note User should ensure this tree is not empty
       */
      const_reference front() const noexcept
      { return *begin(); }

      /**
       * @brief Get a reference to the last element in this tree
       * @note User should ensure this tree is not empty
       */
      reference back() noexcept
      { return *rbegin(); }

      /**
       * @brief Get a const reference to the last element in this tree
       * @note User should ensure this tree is not empty
       */
      const_reference back() const noexcept
      { return *rbegin(); }

      /**
       * @brief Find an element whose index is equals to @p val
       * @tparam LookupPolicy Lookup policy
       * @param val The value of index to be searched for
       * @param p policy
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the nearest one (first touched) will be return. if not found,
       *        @a end() will be returned.
       */
      template<typename LookupPolicy = default_lookup_policy>
      lookup_policy_sfinae<LookupPolicy, iterator>
      find(const Index &val, LookupPolicy p = LookupPolicy())
          noexcept(node_type::is_comparator_noexcept)
      { return find(end(), val, p); }

      /**
       * @brief Find an element whose index is equals to @p val
       * @tparam LookupPolicy Lookup policy
       * @param val The value of index to be searched for
       * @param hint Search is start from this position rather than the root
       *             of tree, may affects performance depends on the position
       *             between result position and @p hint
       * @param p policy
       *
       * @returns
       *        An iterator point to the element with same index value with
       *        @p val, if there are more than one elements have same index,
       *        the nearest one (first touched) will be return. if not found,
       *        @a end() will be returned.
       */
      template<typename LookupPolicy = default_lookup_policy>
      lookup_policy_sfinae<LookupPolicy, iterator>
      find(iterator hint, const Index &val, LookupPolicy p = LookupPolicy())
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        auto *result = node_type::find(ref, val, p);
        if (result == nullptr) return end();
        else return iterator(result);
      }

      /**
       * @brief Find an element whose index is equals to @p val
       * @tparam LookupPolicy Lookup policy
       * @param val The value of index to be searched for
       * @param p policy
       * @returns
       *        A const iterator point to the element with same index value
       *        with @p val, if there are more than one elements have same
       *        index, the one element (first touched) will be return. if not
       *        found, @a end() will be returned.
       */
      template<typename LookupPolicy = default_lookup_policy>
      lookup_policy_sfinae<LookupPolicy, const_iterator>
      find(const Index &val, LookupPolicy p = LookupPolicy()) const
          noexcept(node_type::is_comparator_noexcept)
      { return find(end(), val, p); }

      /**
       * @brief Find an element whose index is equals to @p val
       * @tparam LookupPolicy Lookup policy
       * @param val The value of index to be searched for
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @param p policy
       * @returns
       *        A const iterator point to the element with same index value
       *        with @p val, if there are more than one elements have same
       *        index, the nearest one (first touched) will be return. if not
       *        found, @a end() will be returned.
       */
      template<typename LookupPolicy = default_lookup_policy>
      lookup_policy_sfinae<LookupPolicy, const_iterator>
      find(const_iterator hint, const Index &val, LookupPolicy p = LookupPolicy()) const
          noexcept(node_type::is_comparator_noexcept)
      {
        const node_type &ref = *hint;
        auto *result = node_type::find(ref, val, p);
        if (result == nullptr) return end();
        else return const_iterator(result);
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value of index to be searched for
       * @returns An iterator point to the first element that is not less than
       *          @p val, or @a end() if there is no such element
       */
      iterator lower_bound(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        return lower_bound(end(), val);
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value of index to be searched for
       * @returns A const iterator point to the first element that is not less
       *          than @p val, or @a end() if there is no such element
       */
      const_iterator lower_bound(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return lower_bound(end(), val); }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value of index to be searched for
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns An iterator point to the first element that is not less than
       *          @p val, or @a end() if there is no such element
       */
      iterator lower_bound(iterator hint, const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return iterator(node_type::lower_bound(ref, val));
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value of index to be searched for
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A const iterator point to the first element that is not less
       *          than @p val, or @a end() if there is no such element
       */
      const_iterator lower_bound(const_iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        const node_type &ref = *hint;
        return const_iterator(node_type::lower_bound(ref, val));
      }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value of index to be searched for
       * @returns An iterator point to the first element that is greater than
       *          @p val, or @a end() if there is no such element
       */
      iterator upper_bound(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      { return upper_bound(end(), val); }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value of index to be searched for
       * @returns A const iterator point to the first element that is greater than
       *          @p val, or @a end() if there is no such element
       */
      const_iterator upper_bound(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return upper_bound(end(), val); }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value of index to be searched for
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns An iterator point to the first element that is greater than
       *          @p val, or @a end() if there is no such element
       */
      iterator upper_bound(iterator hint, const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return iterator(node_type::upper_bound(ref, val));
      }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value of index to be searched for
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A const iterator point to the first element that is greater than
       *          @p val, or @a end() if there is no such element
       */
      const_iterator upper_bound(const_iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        const node_type &ref = *hint;
        return const_iterator(node_type::upper_bound(ref, val));
      }

      /**
       * @brief Returns a range of elements whose indexes are equals to @p val
       * @param val The value of index to be searched for
       * @returns A pair of iterator that represents the equal range. The
       *          first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<iterator, iterator> equals_range(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      { return equals_range(end(), val); }

      /**
       * @brief Returns a range of elements whose indexes are equals to @p val
       * @param val The value of index to be searched for
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A pair of iterator that represents the equal range. The
       *          first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<iterator, iterator> equals_range(iterator hint, const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        iterator l = lower_bound(hint, val);
        iterator u = upper_bound(l, val);
        return std::make_pair(std::move(l), std::move(u));
      }

      /**
       * @brief Returns a range of elements whose indexes are equals to @p val
       * @param val The value of index to be searched for
       * @returns A pair of const iterator that represents the equal range.
       *          The first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<const_iterator, const_iterator>
      equals_range(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return equals_range(end(), val); }

      /**
       * @brief Returns a range of elements whose indexes are equals to @p val
       * @param val The value of index to be searched for
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A pair of const iterator that represents the equal range.
       *          The first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<const_iterator, const_iterator>
      equals_range(const_iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        const_iterator l = lower_bound(hint, val);
        const_iterator u = upper_bound(l, val);
        return std::make_pair(std::move(l), std::move(u));
      }

      /**
       * @brief Insert an element into this tree
       * @tparam InsertPolicy Insert policy
       * @param val The element will be inserted
       * @param p policy
       * @returns If the element is successfully inserted into this tree,
       *          the iterator for @p e is returned, otherwise, the
       *          iterator for the element which conflict with this element
       *          is returned
       */
      template<typename InsertPolicy = default_insert_policy>
      insert_policy_sfinae<InsertPolicy, iterator>
      insert(value_type &val, InsertPolicy p = InsertPolicy())
          noexcept(node_type::is_comparator_noexcept)
      { return insert(end(), val, p); }

      /**
       * @brief Insert an element into this tree
       * @tparam InsertPolicy Insert policy
       * @param val The element will be inserted
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @param p policy
       * @returns If the element is successfully inserted into this tree,
       *          the iterator for @p e is returned, otherwise, the
       *          iterator for the element which conflict with this element
       *          is returned
       */
      template<typename InsertPolicy = default_insert_policy>
      insert_policy_sfinae<InsertPolicy, iterator>
      insert(iterator hint, value_type &val, InsertPolicy p = InsertPolicy())
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return iterator(node_type::insert(ref, val, p));
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with unique policy
       * @tparam InputIterator the type of the iterator
       * @tparam InsertPolicy Insert policy
       * @param b The begin of the range
       * @param e The end of the range
       * @param p policy
       */
      template<typename InputIterator, typename InsertPolicy = default_insert_policy>
      insert_policy_sfinae<InsertPolicy>
      insert(InputIterator b, InputIterator e, InsertPolicy p = InsertPolicy())
          noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(*b++, p);
      }

      /**
       * @brief Insert all elements from iterator range [\p b, \p e) into this
       *        tree, with unique policy
       * @tparam InputIterator the type of the iterator
       * @tparam InsertPolicy Insert policy
       * @param b The begin of the range
       * @param e The end of the range
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @param p policy
       */
      template<typename InputIterator,
        typename InsertPolicy = default_insert_policy>
      insert_policy_sfinae<InsertPolicy>
      insert(InputIterator b, InputIterator e, iterator hint,
          InsertPolicy p = InsertPolicy())
          noexcept(node_type::is_comparator_noexcept)
      {
        while (b != e)
          insert(hint, *b++, p);
      }

      /** @brief Remote an element that the iterator point to from this tree */
      void erase(iterator iter) noexcept
      {
        node_type &ref = *iter;
        ref.node_type::unlink();
      }

      /**
       * @brief Remote all element inside the range of [@p b, @p e) from
       *        this tree
       */
      void erase(iterator b, iterator e) noexcept
      {
        for (auto i = b; i != e; )
          erase(i++);
      }

      /**
       * @brief Remove all elements that their index are equals to @p val from
       *        this tree
       * @param val The value of index for searching elements
       */
      void erase(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        auto b = lower_bound(end(), val);
        auto e = upper_bound(b, val);
        erase(b, e);
      }

      /** @brief Count the number of node has given index value */
      size_type count(const Index &val) const noexcept
      {
        auto p = equals_range(val);
        size_type ret = 0;
        while (p.first != p.second)
        {
          ++p.first;
          ++ret;
        }
        return ret;
      }

      /** @brief Swap all elements with another tree @p t */
      void swap(rbtree &t) noexcept
      {
        rbtree tmp(std::move(t));
        t = std::move(*this);
        *this = std::move(tmp);
      }

      /** @brief Remove all elements from this tree */
      void clear() noexcept
      { m_container_node.unlink_container(); }

    private:
      rbtree_node<void, void>::container<Index, Node, Tag> m_container_node;
    };

    template<typename Index, typename Node, typename Tag>
    inline bool operator == (const rbtree<Index, Node, Tag> &l
        , const rbtree<Index, Node, Tag> &r)
        noexcept(rbtree_node<Index, Node, Tag>::is_comparator_noexcept)
    {
      auto i = l.begin(), j = r.begin(), m = l.end(), n = r.end();
      while (i != m && j != n && *i == *j)
      {
        i++;
        j++;
      }
      return i == m && j == n;
    };

    template<typename Index, typename Node, typename Tag>
    inline bool operator != (const rbtree<Index, Node, Tag> &l
        , const rbtree<Index, Node, Tag> &r)
        noexcept(rbtree_node<Index, Node, Tag>::is_comparator_noexcept)
    {
      return !(l == r);
    };

    template<typename Index, typename Node, typename Tag>
    inline bool operator < (const rbtree<Index, Node, Tag> &l
        , const rbtree<Index, Node, Tag> &r)
        noexcept(rbtree_node<Index, Node, Tag>::is_comparator_noexcept)
    {
      if (&l == &r) return false;
      return std::lexicographical_compare(l.begin(), l.end(),
                                          r.begin(), r.end());
    };

    template<typename Index, typename Node, typename Tag>
    inline bool operator <= (const rbtree<Index, Node, Tag> &l
        , const rbtree<Index, Node, Tag> &r)
        noexcept(rbtree_node<Index, Node, Tag>::is_comparator_noexcept)
    {
      return !(r < l);
    };

    template<typename Index, typename Node, typename Tag>
    inline bool operator > (const rbtree<Index, Node, Tag> &l
        , const rbtree<Index, Node, Tag> &r)
        noexcept(rbtree_node<Index, Node, Tag>::is_comparator_noexcept)
    {
      return r < l;
    };

    template<typename Index, typename Node, typename Tag>
    inline bool operator >= (const rbtree<Index, Node, Tag> &l
        , const rbtree<Index, Node, Tag> &r)
        noexcept(rbtree_node<Index, Node, Tag>::is_comparator_noexcept)
    {
      return !(l < r);
    };

  }
}
