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


#ifndef LANXC_INTRUS_RBTREE_HPP_INCLUDED
#define LANXC_INTRUS_RBTREE_HPP_INCLUDED

#include "rbtree_config.hpp"
#include "rbtree_node.hpp"
#include "rbtree_iterator.hpp"

#include <iterator>
#include <type_traits>

namespace lanxc
{
  namespace intrus
  {

    /**
     * @brief RBTree container
     * @ingroup intrusive_rbtree
     */
    template<typename Index, typename Node, typename Tag>
    class rbtree
    {
      using detail                  = rbtree_node<void, void>;
      using node_type               = rbtree_node<Index, Node, Tag, detail>;
      using config                  = typename node_type::config;
      using default_insert_policy   = typename config::default_insert_policy;
      using default_lookup_policy   = typename config::default_lookup_policy;


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
      using const_pointer          = const value_type &;
      using const_reference        = const value_type *;
      using size_type              = std::size_t;
      using difference_type        = std::ptrdiff_t;

      rbtree() noexcept
        : m_container_node(node_type::container)
      { }

      ~rbtree() noexcept
      { clear(); };

      rbtree(const rbtree &) = delete;

      rbtree(rbtree &&t) noexcept
        : m_container_node(std::move(t.m_container_node))
      {  }

      rbtree &operator = (const rbtree &t) = delete;

      rbtree &operator = (rbtree &&t) noexcept
      {
        if (&t != this)
        {
          this->~rbtree();
          new (this) rbtree(std::move(t));
        }
        return *this;
      }


      /** @brief Test if this rbtree is empty */
      bool empty() const noexcept
      { return m_container_node.is_empty_container_node(); }


      /** @brief Count the elements in this tree */
      size_type size() const noexcept
      {
        size_type s = 0;
        auto b = begin(), e = end();
        while (++b != e) ++s;
        return s;
      }

      iterator begin() noexcept
      { return iterator(m_container_node.front_of_container()); }

      /**
       * @brief Get an reverse iterator to the position of the last element in
       *         this tree
       */
      reverse_iterator rbegin() noexcept
      { return reverse_iterator(end()); }

      /**
       * @brief Get an const iterator point to the position of the first element
       *        in this tree
       */
      const_iterator begin() const noexcept
      { return const_iterator(end()); }

      /**
       * @brief Get an const reverse iterator to the position of the last
       *        element in this tree
       */
      const_reverse_iterator rbegin() const noexcept
      { return const_reverse_iterator(end()); }

      /**
       * @brief Get an const iterator point to the position of the first
       *        element in this tree
       */
      const_iterator cbegin() const noexcept
      { return const_iterator(m_container_node.front_of_container()); }

      /**
       * @brief Get an const reverse iterator to the position of the last
       *        element in this tree
       */
      const_reverse_iterator crbegin() const noexcept
      { return const_reverse_iterator(end()); }

      /**
       * @brief Get an iterator to the position after the last element
       *        in this tree
       */
      iterator end() noexcept
      { return iterator(&m_container_node); }

      /**
       * @brief Get an reverse iterator point to the position before the
       *        first element in this tree
       */
      reverse_iterator rend() noexcept
      { return reverse_iterator(begin()); }

      /**
       * @brief Get an const iterator point to the position after the last
       *        element in this tree
       */
      const_iterator end() const noexcept
      { return const_iterator(&m_container_node); }

      /**
       * @brief Get an const reverse iterator point to the position before the
       *        first element in this tree
       */
      const_reverse_iterator rend() const noexcept
      { return const_reverse_iterator(begin()); }

      /**
       * @brief Get an const iterator point to the position after the last
       *        element in this tree
       */
      const_iterator cend() const noexcept
      { return const_iterator(&m_container_node); }

      /**
       * @brief Get an const reverse iterator point to the position before the
       *        first element in this tree
       */
      const_reverse_iterator crend() const noexcept
      { return const_reverse_iterator(begin()); }

      /**
       * @brief Get a reference to the first element in this tree
       * @note User should ensure this tree is not empty
       */
      reference front() noexcept
      { return *begin(); }

      /**
       * @brief Get a const reference to the first element in this tree
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
       * @brief Find an element its index is equals to @p val
       * @tparam LookupPolicy Lookup policy
       * @param val The value of index of the element want to find
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
       * @brief Find an element its index is equals to @p val
       * @tparam LookupPolicy Lookup policy
       * @param val  The value of index of the element want to find
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
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
       * @brief Find an element its index is equals to @p val
       * @tparam LookupPolicy Lookup policy
       * @param val The value of index of the element want to find
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
       * @brief Find an element its index is equals to @p val
       * @tparam LookupPolicy Lookup policy
       * @param val The value of index of the element want to find
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
        node_type &ref = *hint;
        auto *result = node_type::find(ref, val, p);
        if (result == nullptr) return end();
        else return const_iterator(result);
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @returns An iterator point to the first element that is not less than
       *          @p val
       */
      iterator lower_bound(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        return lower_bound(end(), val);
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @returns A const iterator point to the first element that is not less
       *          than @p val
       */
      const_iterator lower_bound(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return lower_bound(end(), val); }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns An iterator point to the first element that is not less than
       *          @p val
       */
      iterator lower_bound(iterator hint, const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return iterator(node_type::lower_bound(ref, val));
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A iterator point to the first element that is not less than
       *          @p val
       */
      iterator lower_bound(iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return const_iterator(node_type::lower_bound(ref,
              node_type::get_val(val)));
      }

      /**
       * @brief Find the lower bound for @p val in this tree
       * @param val The value for searching the boundary
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A const iterator point to the first element that is not less
       *          than @p val
       */
      const_iterator lower_bound(const_iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        node_type &ref = *hint;
        return const_iterator(node_type::lower_bound(ref, val));
      }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value for searching the boundary
       * @returns An iterator point to the first element that is greater than
       *          @p val
       */
      iterator upper_bound(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      { return upper_bound(end(), val); }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value for searching the boundary
       * @returns A const iterator point to the first element that is greater than
       *          @p val
       */
      const_iterator upper_bound(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return upper_bound(end(), val); }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value for searching the boundary
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns An iterator point to the first element that is greater than
       *          @p val
       */
      iterator upper_bound(iterator hint, const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      {
        auto &ref = hint->template get_node<Tag>();
        return iterator(node_type::upper_bound(ref, val));
      }

      /**
       * @brief Find the upper bound for @p val in this tree
       * @param val The value for searching the boundary
       * @param hint Search is start from this position other than the root
       *             of tree, may affects performance depends on the position
       *             between search result and @p hint
       * @returns A const iterator point to the first element that is greater than
       *          @p val
       */
      const_iterator upper_bound(const_iterator hint, const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      {
        auto &ref = hint->template get_node<Tag>();
        return const_iterator(node_type::upper_bound(ref, val));
      }

      /**
       * @brief Returns a range that indexes of elements inside are equals to
       *        @p val
       * @param val The value for searching the equal range
       * @returns A pair of iterator that represents the equal range. The
       *          first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<iterator, iterator> equals_range(const Index &val)
          noexcept(node_type::is_comparator_noexcept)
      { return equals_range(end(), val); }

      /**
       * @brief Returns a range that indexes of elements inside are equals to
       *        @p val
       * @param val The value for searching the equal range
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
        auto l = lower_bound(hint, val);
        auto u = upper_bound(l, val);
        return std::make_pair(std::move(l), std::move(u));
      }

      /**
       * @brief Returns a range that indexes of elements inside are equals to
       *        @p val
       * @param val The value for searching the equal range
       * @returns A pair of const iterator that represents the equal range.
       *          The first iterator is the lower bound, the second iterator
       *          is the upper bound
       */
      std::pair<const_iterator, const_iterator>
      equals_range(const Index &val) const
          noexcept(node_type::is_comparator_noexcept)
      { return equals_range(end(), val); }

      /**
       * @brief Returns a range that indexes of elements inside are equals to
       *        @p val
       * @param val The value for searching the equal range
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
        auto l = lower_bound(hint, val);
        auto u = upper_bound(l, val);
        return std::make_pair(std::move(l), std::move(u));
      }

      /**
       * @brief Insert an element into this tree
       * @tparam InsertPolicy Insert policy
       * @param val The element will be inserted
       * @param p policy
       * @returns If the element is successfuly inserted into this tree,
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
       * @returns If the element is successfuly inserted into this tree,
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
      insert_policy_sfinae<InsertPolicy, iterator>
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
      void swap(rbtree &&t) noexcept
      { swap(t); }

      /** @brief Swap all elements with another tree @p t */
      void swap(rbtree &t) noexcept
      { node_type::swap_nodes(m_container_node, t.m_container_node); }

      void clear() noexcept
      { m_container_node.unlink_container(); }

    private:
      node_type m_container_node;
    };
  }
}
#endif

