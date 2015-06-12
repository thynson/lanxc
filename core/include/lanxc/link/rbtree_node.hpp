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

#ifndef LANXC_INTRUS_RBTREE_NODE_HPP_INCLUDED
#define LANXC_INTRUS_RBTREE_NODE_HPP_INCLUDED

#include "rbtree_config.hpp"

namespace lanxc
{
  namespace link
  {


    template<>
    class rbtree_node<void, void>
    {

      template<typename, typename, typename...>
      friend class rbtree_node;

      template<typename, typename, typename>
      friend class rbtree;

      /**
       * @brief SFINAE check for lookup policy
       * @tparam Policy Type of lookup policy
       * @tparam Result SFINAE Result
       */
      template<typename Policy, typename Result = void>
      using lookup_policy_sfinae    = typename std::enable_if<
          index_policy::is_lookup_policy<Policy>::value,
          Result>::type;

      /**
       * @brief SFINAE check for insert policy
       * @tparam Policy Type of insert policy
       * @tparam Result SFINAE Result
       */
      template<typename Policy, typename Result = void>
      using insert_policy_sfinae    = typename std::enable_if<
          index_policy::is_insert_policy<Policy>::value,
          Result>::type;

    };

    template<typename Index>
    class rbtree_node<Index, void>
    {
      template<typename, typename, typename...>
      friend class rbtree_node;

      template<typename, typename, typename>
      friend class rbtree_iterator;

      template<typename, typename, typename>
      friend class rbtree_const_iterator;

      template<typename, typename, typename>
      friend class rbtree;


      template<typename ...Arguments>
      rbtree_node(Arguments &&...arguments)
        noexcept(noexcept(Index(std::forward<Arguments>(arguments)...)))
        : m_index (std::forward<Arguments>(arguments)...)
      {}

      Index m_index;
    };


    template<typename Index, typename Node, typename Tag>
    class rbtree_node<Index, Node, rbtree_config<Tag>>
    {
    public:
      using pointer = rbtree_node *;
      using const_pointer = const rbtree_node *;
      using reference = rbtree_node &;
      using const_reference = const rbtree_node &;

      using config = rbtree_config<Tag>;
      using comparator_type = typename config::template comparator<Index>;
      using node_pointer = typename config::template node_pointer<rbtree_node>;

      static struct container_tag {} container;
      static constexpr bool is_comparator_noexcept =
        noexcept(std::declval<typename config::template comparator<Index>>()
            (std::declval<Index>(), std::declval<Index>()));

      rbtree_node() noexcept
        : m_p(nullptr), m_l(nullptr), m_r(nullptr)
        , m_is_red(false), m_is_container(false)
        , m_has_l(false), m_has_r(false)
      { }

      ~rbtree_node() noexcept
      {
        if (m_is_container)
          unlink_container();
        else
          unlink();
      }

      rbtree_node(rbtree_node &&n) noexcept
        : rbtree_node()
      { move(*this, n); }

      rbtree_node &operator = (rbtree_node &&n) noexcept
      {
        if (this != &n)
        {
          this->~rbtree_node();
          new (this) rbtree_node(std::move(n));
        }
        return *this;
      }


      /** @brief Test if this node is linked into rbtree */
      bool is_linked() const noexcept
      { return m_p != nullptr; }


      /**
       * @brief Unlink this node from a tree or destroy a tree if this is the
       * container node
       * @returns return value of next() before destroyed, or nullptr if this
       * node is container node or a node already unlinked
       **/
      bool unlink() noexcept
      {
        return unlink_and_get_adjoin_node().first != nullptr;
      }


	    const Index &get_index() const noexcept
	    {
		    const Node &n = static_cast<const Node &>(*this);
		    return n.rbtree_node<Index, void>::m_index;
	    }

    private:

	    rbtree_node(container_tag) noexcept
			    : m_p(this), m_l(this), m_r(this)
			    , m_is_red(true), m_is_container(true)
			    , m_has_l(false), m_has_r(false)
	    {}

	    Index &internal_get_index() noexcept
	    {
		    Node &n = static_cast<Node&>(*this);
		    return n.rbtree_node<Index, void>::m_index;
	    }

      /** @brief test if a node is container node or root node */
      bool is_container_or_root() const noexcept
      { return m_p != nullptr && m_p->m_p == this; }

      /** @brief Test if a node is the root node of rbtree */
      bool is_root_node() const noexcept
      { return is_container_or_root() && !this->m_is_container; }

      /** @brief Test if container is empty */
      bool is_empty_container_node() const noexcept
      { return m_p == this; }

      /**
       * @brief Get root node from container node
       * @note User is responsible to ensure this node is container node
       * @returns the root node of this rbtree, or nullptr if this tree is an
       * empty tree
       */
      const_pointer get_root_node_from_container_node() const noexcept
      {
        if (m_p == this)
          return nullptr;
        else
          return m_p;
      }

      /**
       * @brief Get root node from container node
       * @note User is responsible to ensure this node is container node
       * @returns the root node of this rbtree, or nullptr if this tree is an
       * empty tree
       */
      pointer get_root_node_from_container_node() noexcept
      {
        if (m_p == this)
          return nullptr;
        else
          return m_p;
      }

      /** @brief Get root node */
      const_pointer get_root_node() const noexcept
      {
        if (m_is_container)
          if (m_p == this)
            return nullptr;
        pointer p = this;
        while (p->m_p->m_p != p) p = p->m_p;
        return p;
      }

      /** @brief Get root node */
      pointer get_root_node() noexcept
      {
        if (m_is_container)
          if (m_p == this)
            return nullptr;
        pointer p = this;
        while (p->m_p->m_p != p) p = p->m_p;
        return p;
      }

      pointer get_container_node() noexcept
      {
        if (m_is_container) return this;
        if (m_p == nullptr) return nullptr;
        auto p = this;
        while (!p->m_is_container) p = p->m_p;
        return p;
      }

      const_pointer get_container_node() const noexcept
      {
        if (m_is_container) return this;
        if (m_p == nullptr) return nullptr;
        auto p = this;
        while (!p->m_is_container) p = p->m_p;
        return p;
      }

      const_pointer front_of_container() const noexcept
      { return m_l; }

      const_pointer back_of_container() const noexcept
      { return m_r; }

      pointer front_of_container() noexcept
      { return m_l; }

      pointer back_of_container() noexcept
      { return m_r; }

      /** @brief Get front node of this subtree */
      pointer front() noexcept
      {
        pointer p = this;
        while (p->m_has_l) p = p->m_l;
        return p;
      }

      /** @brief Get front node of this subtree */
      const_pointer front() const noexcept
      {
        const_pointer p = this;
        while (p->m_has_l) p = p->m_l;
        return p;
      }

      pointer back() noexcept
      {
        pointer p = this;
        while (p->m_has_r) p = p->m_r;
        return p;
      }

      const_pointer back() const noexcept
      {
        const_pointer p = this;
        while (p->m_has_r) p = p->m_r;
        return p;
      }

      pointer next() noexcept
      {
        if (m_is_container) return m_l;
        if (m_has_r) return m_r->front();
        else return m_r;
      }

      pointer prev() noexcept
      {
        if (m_is_container) return m_r;
        if (m_has_l) return m_l->back();
        else return m_l;
      }

      const_pointer next() const noexcept
      {
        if (m_is_container) return m_l;
        if (m_has_r) return m_r->front();
        else return m_r;
      }

      const_pointer prev() const noexcept
      {
        if (m_is_container) return m_r;
        if (m_has_l) return m_l->back();
        else return m_l;
      }

      /**
       * @brief Swap two nodes
       * @note This will generally break the order or nodes, so it's declared
       * privately
       */
      static void swap_nodes(reference lhs, reference rhs) noexcept
      {
        if (&lhs == &rhs) return;
        rbtree_node tmp;
        move(tmp, lhs);
        move(lhs, rhs);
        move(rhs, tmp);
      }

      /**
       * @brief Transfer then linkship from @p src to @p dst
       */
      static void move(reference dst, reference src) noexcept
      {
        dst.~rbtree_node();

        if (src.m_is_container)
        {
          new (&dst) rbtree_node(container);
          if (src.is_empty_container_node())
          {
            src.unlink_cleanup();
            return;
          }
        }
        else
        {
          new (&dst) rbtree_node();
        }

        if (src.is_linked())
        {

          dst.m_p = src.m_p;
          dst.m_l = src.m_l;
          dst.m_r = src.m_r;
          dst.m_has_l = src.m_has_l;
          dst.m_has_r = src.m_has_r;
          dst.m_is_red = src.m_is_red;

          if (src.m_is_container)
          {
            dst.m_p->m_p = &dst;
            dst.m_l->m_l = &dst;
            dst.m_r->m_r = &dst;
            src.unlink_cleanup();
            return;
          }

          if (src.m_p->m_is_container)
            src.m_p->m_p = &dst;
          else if (&src == src.m_p->m_l)
            src.m_p->m_l = &dst;
          else
            src.m_p->m_r = &dst;

          if (src.m_has_l)
          {
            src.m_l->m_p = &dst;
            src.m_l->back()->m_r = &dst;
          }
          else if (src.m_l->m_is_container)
            dst.m_l->m_l = &dst;

          if (src.m_has_r)
          {
            src.m_r->m_p = &dst;
            src.m_r->front()->m_l = &dst;
          }
          else if (src.m_r->m_is_container)
            src.m_r->m_r = &dst;

          src.unlink_cleanup();
        }
      }

      void lrotate() noexcept
      {
        pointer y = m_r;
        if (y->m_has_l)
        {
          m_r = y->m_l;
          m_r->m_p = this;
        }
        else
        {
          m_has_r = false;
          m_r = y;
          y->m_has_l = true;
        }

        y->m_p = m_p;
        if (m_p->m_p == this) m_p->m_p = y;
        else if (m_p->m_l == this) m_p->m_l = y;
        else m_p->m_r = y;

        y->m_l = this;
        m_p = y;
      }

      void rrotate() noexcept
      {
        pointer y = m_l;
        if (y->m_has_r)
        {
          m_l = y->m_r;
          m_l->m_p = this;
        }
        else
        {
          m_has_l = false;
          m_l = y;
          y->m_has_r = true;
        }

        y->m_p = m_p;
        if (m_p->m_p == this) m_p->m_p = y;
        else if (m_p->m_r == this) m_p->m_r = y;
        else m_p->m_l = y;

        y->m_r = this;
        m_p = y;
      }

      /** @brief Rebalance a node after insertion */
      static void
      rebalance_for_insertion(pointer node) noexcept
      {
        while(node->m_p->m_is_red && !node->is_container_or_root())
          // Check node is not root of node and its parent are red
        {
          pointer parent = node->m_p;

          if (parent == parent->m_p->m_l)
          {
            if (parent->m_p->m_has_r && parent->m_p->m_r->m_is_red)
            {
              pointer y = parent->m_p->m_r;
              parent->m_is_red = false;

              y->m_is_red = false;
              parent->m_p->m_is_red = true;
              node = parent->m_p;
              parent = node->m_p;
            }
            else
            {
              if (parent->m_r == node)
              {
                node = parent;
                node->lrotate();
                parent = node->m_p;
              }

              parent->m_p->rrotate();
              parent->m_is_red = false;
              parent->m_r->m_is_red = true;
            }
          }
          else if (parent == parent->m_p->m_r)
          {
            if (parent->m_p->m_has_l && parent->m_p->m_l->m_is_red)
            {
              pointer y = parent->m_p->m_l;
              parent->m_is_red = false;
              y->m_is_red = false;
              parent->m_p->m_is_red = true;
              node = parent->m_p;
              parent = node->m_p;
            }
            else
            {
              if (parent->m_l == node)
              {
                node = parent;
                node->rrotate();
                parent = node->m_p;
              }

              parent->m_p->lrotate();
              parent->m_is_red = false;
              parent->m_l->m_is_red = true;

            }
          }
        }

        if (node->is_container_or_root())
          node->m_is_red = false;
        node = node->get_container_node();
        static_cast<rbtree_node<void, void, rbtree<Index, Node, Tag>>*>(node)->m_size++;
      }

      /**
       * @brief Rebalance the tree for unlink operation
       * @returns The container node
       */
      static pointer
      rebalance_for_unlink(pointer node) noexcept
      {

        // Be careful that node may have detached from the tree
        while (node->m_is_red == false && !node->is_container_or_root())
        {
          pointer parent = node;

          if (parent->m_l == node)
          {

            pointer w = parent->m_r;
            if (w->m_is_red)
              // case 1:
            {
              parent->lrotate();
              parent->m_is_red = true;
              parent->m_p->m_is_red = false;
              w = parent->m_r;
            }


            if ((!w->m_has_l || w->m_l->m_is_red == false)
                && (!w->m_has_r || w->m_r->m_is_red == false))
            // case 2:
            {
              w->m_is_red = true;
              node = parent;
              parent = node->m_p;
            }
            else
            {
              if (!w->m_has_r || w->m_r->m_is_red == false)
                // case 3:
              {
                w->rrotate();
                w->m_p->m_is_red = false;
                w->m_is_red = true;
                w = parent->m_r;
              }

              // case 4:

              w->m_is_red = parent->m_is_red;
              parent->lrotate();
              parent->m_is_red = false;
              w->m_r->m_is_red = false;
              break;
            }
          }
          else if (parent->m_r == node)
          {

            pointer w = parent->m_l;
            if (w->m_is_red)
              // case 1:
            {
              parent->rrotate();
              parent->m_is_red = true;
              parent->m_p->m_is_red = false;
              w = parent->m_l;
            }


            if ((!w->m_has_l || w->m_l->m_is_red == false)
                && (!w->m_has_r || w->m_r->m_is_red == false))
            // case 2:
            {
              w->m_is_red = true;
              node = parent;
              parent = node->m_p;
            }
            else
            {
              if (!w->m_has_l || w->m_l->m_is_red == false)
                // case 3:
              {
                w->lrotate();
                w->m_is_red = true;
                w->m_p->m_is_red = false;
                w = parent->m_l;
              }

              // case 4:

              w->m_is_red = parent->m_is_red;
              parent->rrotate();
              parent->m_is_red = false;
              w->m_l->m_is_red = false;
              break;
            }
          }
          else
            break;
        }

        if (node->m_is_container)
          return node;
        else if (!node->is_root_node())
          node = node->get_root_node();
        node->m_is_red = false;
        return node->get_container_node();
      }

      /**
       * @brief Insert a node as left child of this node
       * @note User is responsible for ensure this node does not have left
       * child
       */
      void insert_as_left_child(pointer node) noexcept
      {
        node->m_l = m_l;
        if (m_l->m_is_container)
          m_l->m_l = node;
        node->m_r = this;
        node->m_p = this;
        m_l = node;
        m_has_l = true;
        node->m_is_red = true;
        rebalance_for_insertion(node);
      }

      /**
       * @brief Insert a node as right child of this node
       * @note User is responsible for ensure this node does not have right
       * child
       */
      void insert_as_right_child(pointer node) noexcept
      {
        node->m_r = m_r;
        if (m_r->m_is_container)
          m_r->m_r = node;
        node->m_l = this;
        node->m_p = this;
        m_r = node;
        m_has_r = true;
        node->m_is_red = true;
        rebalance_for_insertion(node);
      }

      /**
       * @brief Insert a node as root node
       * @note User is responible to ensure this node is container node
       */
      void insert_root_node(pointer node) noexcept
      {
        m_p = m_l = m_r = node;
        node->m_p = node->m_l = node->m_r = this;
        rebalance_for_insertion(node);
      }

      /** @brief Insert a node as predecessor of this node */
      void insert_before (pointer node) noexcept
      {
        node->unlink();

        if (m_is_container)
          insert_root_node(node);
        else if (m_has_l)
          prev()->insert_as_left_child(node);
        else
          insert_as_left_child(node);
      }

      /** @brief Insert a node as successor of this node */
      void insert_after(pointer node) noexcept
      {
        node->unlink();
        if (m_is_container)
          insert_root_node(node);
        else if (this->m_has_r)
          next()->insert_as_right_child(node);
        else
          insert_as_right_child(node);
      }


      /**
       * @brief Insert node as parent's chlid, left child or right chlid are
       * all possible.
       * @param entry The node want child
       * @param node The node want to be inserted
       * @note if entry is equals to node, this function will return without
       * any changed
       */
      static void insert(pointer entry, pointer node) noexcept
      {
        if (entry == node)
          return;
        node->unlink();
        if (entry->m_is_container)
          entry->insert_root_node(node);
        else if (!entry->m_has_l)
          entry->insert_as_left_child(node);
        else if (!entry->m_has_r)
          entry->insert_as_right_child(node);
        else
          entry->next()->insert_as_left_child(node);
      }

      /**
       * @brief Insert a node between prev and next
       * @param prev The node will become predecessor of node after insertion
       * is done
       * @param next The node will be successor of node after insertion is
       * done
       * @param node The node want to be inserted
       * @note User code should ensure that #prev is predecessor of #next
       */
      static void insert_between(pointer prev, pointer next,
          pointer node) noexcept
      {
        if (prev == node || next == node)
          return;
        if (prev == next)
          insert(prev, node);
        else if (prev->m_is_container)
          next->insert_as_left_child(node);
        else if (next->m_is_container)
          prev->insert_as_right_child(node);
        else if (prev->m_has_r)
          next->insert_as_left_child(node);
        else
          prev->insert_as_right_child(node);
      }

      /**
       * @brief Insert a node between prev and next in condition that no index
       * will become duplicated
       * @param prev The node will become predecessor of the node after insertion
       * is done
       * @param next The node will be successor of the node after insertion is
       * done
       * @param node The node want to be inserted
       * @note User code should ensure that prev is predecessor of next
       */
      static void insert_conflict(pointer prev, pointer next,
          pointer node) noexcept
      {
        if (prev == node || next == node)
          return;
        if (prev == next)
        {
          if (prev->m_is_container)
            prev->insert_root_node(node);
          else
            ; // Conflict, do nothing
        }
        else
          insert_between(prev, next, node);
      }


      void unlink_cleanup() noexcept
      {
        m_l = m_r = m_p = m_is_container ? this : nullptr;
        m_has_l = m_has_r = false;
        m_is_red = m_is_container;
      }

      bool unlink_container() noexcept
      {
        for (auto p = m_l; p != this; )
        {
          auto current = p;
          p = p->next();
          current->unlink_cleanup();
        }


        m_p = m_l = m_r = this;
        m_has_l = m_has_r = false;
        m_is_red = true;
        static_cast<rbtree_node<void, void, rbtree<Index, Node, Tag>>*>(this)->m_size = 0;
        return false;
      }

      std::pair<pointer, pointer>
      unlink_and_get_adjoin_node() noexcept
      {
        if (!is_linked())
          return std::make_pair(nullptr, nullptr);
        std::pair<pointer, pointer> ret(prev(), next());
        pointer x;

        if (m_has_l && m_has_r)
        {
          if (ret.first->m_is_red)
            swap_nodes(*ret.first, *this);
          else
            swap_nodes(*ret.second, *this);
        }


        if (m_has_l) x = m_l;
        else if (m_has_r) x = m_r;
        else x = this;

        x->m_p = m_p;
        if (m_p->m_is_container)
        {
          if (this == x)
          {
            m_p->m_p = m_p;
            m_p->m_l = m_p;
            m_p->m_r = m_p;
          }
          else
          {
            m_p->m_p = x;
            m_p->m_l = x->front();
            m_p->m_r = x->back();
            if (x == m_l) x->back()->m_r = m_p;
            else x->front()->m_l = m_p;
          }
        }
        else if (this == m_p->m_l)
        {
          if (this == x)
          {
            m_p->m_l = m_l;
            m_p->m_has_l = false;
            if (m_l->m_is_container)
              m_l->m_l = m_p;
          }
          else
          {
            m_p->m_l = x;
            if (x == this->m_r)
            {
              if (m_l->m_is_container)
              {
                m_l->m_l = x->front();
                m_l->m_l->m_l = m_l;
              }
              else x->front()->m_l = m_l;
            }
            else x->back()->m_r = m_p;
          }
        }
        else
        {
          if (this == x)
          {
            m_p->m_r = m_r;
            m_p->m_has_r = false;
            if (m_r->m_is_container)
              m_r->m_r = m_p;
          }
          else
          {
            m_p->m_r = x;
            if (x == this->m_l)
            {
              if (m_r->m_is_container)
              {
                m_r->m_r = x->back();
                m_r->m_r->m_r = m_r;
              }
              else x->back()->m_r = m_r;
            }
            else x->front()->m_l = m_p;
          }
        }

        bool need_rebalance = !m_is_red;

        if (need_rebalance)
          x = rebalance_for_unlink(x);
        else
          x = m_p->get_container_node();
        static_cast<rbtree_node<void, void, rbtree<Index, Node, Tag>>*>(x)->m_size--;
        unlink_cleanup();
        return ret;
      }

      pointer unlink_for_hint() noexcept
      {
        auto pair = unlink_and_get_adjoin_node();
        if (pair.first == nullptr)
          return nullptr;
        if (pair.first->m_is_container)
          return pair.second;
        else
          return pair.first;
      }



      /**
       * @brief Search for a position for @p index
       * @param entry A node via which search will begin
       * @param index The index we want to search for
       * @returns a pair of pointer to @ref rbtree_node, if the two pointer in
       * pair is the same, then @p index is equals to the index of node which
       * the two pointer point to, else if the pair is not the same, compare
       * the first pointer in the pair with the index is ensured to be true,
       * while compare the second pointer in the pair with the index is
       * ensured to be false.
       */
      template<typename Reference>
      static auto
      search(Reference &entry, const Index &index) noexcept(is_comparator_noexcept)
        -> std::pair<decltype(std::addressof(entry)),
              decltype(std::addressof(entry))>
      {
        auto *p = &entry;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
            return std::make_pair(p, p);
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmp = [&] (Reference &node) noexcept -> bool
        { return s_comparator(node.get_index(), index); };

        auto rcmp = [&] (Reference &node) noexcept -> bool
        { return s_rcomparator(node.get_index(), index); };

        auto result = cmp(*p);
        auto rresult = rcmp(*p);

        if (result != rresult)
          return std::make_pair(p, p);
        else if (result)
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_l)
            {
              auto x = cmp(*p->m_p);
              auto y = rcmp(*p->m_p);
              if (x != y)
                return std::make_pair(p->m_p, p->m_p);
              else if (x)
                p = p->m_p;
              else
                break;
            }
            else if (!p->m_r->m_is_container)
            {
              auto x = cmp(*p->m_r);
              auto y = rcmp(*p->m_r);
              if (x != y)
                return std::make_pair(p->m_r, p->m_r);
              if (x)
                p = p->m_r;
              else
              {
                p = p->m_r;
                result = x;
                break;
              }
            }
            else
              return std::make_pair(p, p->m_r);
          }
        }
        else
        {
          while (!p->m_p->m_is_container)
          {
            if (p == p->m_p->m_r)
            {
              auto x = cmp(*p->m_p);
              auto y = rcmp(*p->m_p);
              if (x != y)
                return std::make_pair(p->m_p, p->m_p);
              else if (!x)
                p = p->m_p;
              else
                break;
            }
            else if (!p->m_l->m_is_container)
            {
              auto x = cmp(*p->m_l);
              auto y = rcmp(*p->m_l);
              if (x != y)
                return std::make_pair(p->m_l, p->m_l);
              if (!x)
                p = p->m_l;
              else
              {
                p = p->m_l;
                result = x;
                break;
              }
            }
            else
              return std::make_pair(p->m_l, p);
          }
        }

        for ( ; ; )
        {
          if (result)
            if (p->m_has_r)
              p = p->m_r;
            else
              return std::make_pair(p, p->m_r);
          else
            if (p->m_has_l)
              p = p->m_l;
            else
              return std::make_pair(p->m_l, p);

          result = cmp(*p);
          rresult = rcmp(*p);
          if (result != rresult)
            return std::make_pair(p, p);
        }
      }

      /**
       * @brief Find the boundry defined by @p index and @p comparator
       * @tparam Reference Type of reference to rbtree_node, should be
       * `rbtree_node &` or `const rbtree_node &`.
       * @tparam Comparator Type of comparator
       * @param entry The entry node via which the tree will be searched
       * @param index The specified index depends on which the boundry is find
       * @param comparator The comparator used to define the boundry
       *
       * The boundry is defined in such way:  each node in the left part
       * has a index value `v` where `comparator(v, index)` is `true`; while
       * each node in right part has index value `v` where
       * `comparator(v, * index)` is `false`
       */
      template<typename Reference, typename Comparator>
      static auto boundry(Reference &entry, const Index &index,
          const Comparator &comparator)
        noexcept(is_comparator_noexcept)
        -> std::pair<decltype(&entry), decltype(&entry)>
      {
        auto *p = &entry;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
            return std::make_pair(p, p);
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmper = [&index, &comparator] (Reference &node) noexcept -> bool
        { return comparator(node.get_index(), index); };

        bool hint_result = cmper(*p);

        if (hint_result)
        {
          while (!p->is_container_or_root())
          {
            if (p == p->m_p->m_l)
            {
              if (cmper(*p->m_p))
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_r->m_is_container)
            {
              if (cmper(*p->m_r)) p = p->m_r;
              else break;
            }
            else
            {
              return std::make_pair(p, p->m_r);
            }
          }
        }
        else
        {
          while (!p->is_container_or_root())
          {
            if (p == p->m_p->m_r)
            {
              if (!cmper(*p->m_p))
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_l->m_is_container)
            {
              if (!cmper(*p->m_l)) p = p->m_l;
              else break;
            }
            else
            {
              return std::make_pair(p->m_l, p);
            }
          }
        }

        for ( ; ; )
        {
          if (hint_result)
          {
            if (p->m_has_r) p = p->m_r;
            else return std::make_pair(p, p->m_r);
          }
          else
          {
            if (p->m_has_l) p = p->m_l;
            else return std::make_pair(p->m_l, p);
          }
          hint_result = cmper(*p);
        }
      }

      static const_pointer find(const_reference entry,
          const Index &index, index_policy::backmost) noexcept(is_comparator_noexcept)
      {
        auto *p = boundry(entry, index, s_rcomparator).first;

        if (s_comparator(p->get_index(), index) != s_rcomparator(p->get_index(), index))
          return p;
        else
          return nullptr;
      }

      static const_pointer find(const_reference entry,
          const Index &index, index_policy::frontmost) noexcept(is_comparator_noexcept)
      {
        auto *p = boundry(entry, index, s_comparator).second;

        if (s_comparator(p->get_index(), index) != s_rcomparator(p->get_index(), index))
          return p;
        else
          return nullptr;
      }

      static const_pointer find(const_reference entry,
          const Index &index, index_policy::nearest) noexcept(is_comparator_noexcept)
      {
        auto p = search(entry, index);

        if (p.first == p.second)
          return p.first;
        else
          return nullptr;
      }

      static pointer find(reference entry,
          const Index &index, index_policy::backmost) noexcept(is_comparator_noexcept)
      {
        auto *p = boundry(entry, index, s_rcomparator).first;

        if (s_comparator(p->get_index(), index)
		        != s_rcomparator(p->get_index(), index))
          return p;
        else
          return nullptr;
      }

      static pointer find(reference entry,
          const Index &index, index_policy::frontmost) noexcept(is_comparator_noexcept)
      {
        auto *p = boundry(entry, index, s_comparator).second;

        if (s_comparator(p->get_index(), index)
		        != !s_rcomparator(p->get_index(),index))
          return p;
        else
          return nullptr;
      }

      static pointer find(reference entry,
          const Index &index, index_policy::nearest) noexcept(is_comparator_noexcept)
      {
        auto p = search(entry, index);

        if (p.first == p.second)
          return p.first;
        else
          return nullptr;
      }

      /**
       * @brief Get the first node in the tree whose index is not less than
       * specified value
       * @param entry Entry node for search
       * @param index The specified value for searching the lower boundry
       */
      static pointer lower_bound(reference entry,
          const Index &index) noexcept(is_comparator_noexcept)
      { return boundry(entry, index, s_comparator).second; }

      /**
       * @brief Get the first node in the tree whose index is not less than
       * specified value
       * @param entry Entry node for search
       * @param index The specified value for searching the lower boundry
       */
      static const_pointer lower_bound(const_reference entry,
          const Index &index) noexcept(is_comparator_noexcept)
      { return boundry(entry, index, s_comparator).second; }

      /**
       * @brief Get the first node in the tree whose index is greater than
       * specified value
       * @param entry Entry node for search
       * @param index The specified value for searching the upper boundry
       */
      static pointer upper_bound(reference &entry,
          const Index &index) noexcept(is_comparator_noexcept)
      { return boundry(entry, index, s_rcomparator).second; }

      /**
       * @brief Get the first node in the tree whose index is greater than
       * specified value
       * @param entry Entry node for search
       * @param index The specified value for searching the upper boundry
       */
      static const_pointer upper_bound(const_reference entry,
          const Index &index) noexcept(is_comparator_noexcept)
      { return boundry(entry, index, s_rcomparator).second; }

      /**
       * @brief Insert @p n to a tree via @p e
       * @param e The entry node for the tree into which @p n will be inserted
       * @param n The node to be inserted
       * @returns pointer to @p n
       * @note User code is responsible to ensure @p e is already attached to
       * an rbtree or is the container node of an rbtree. If index of @n
       * duplicates others, @n will be put in a place behind them
       */
      static pointer insert(reference e, reference n,
          index_policy::backmost) noexcept(is_comparator_noexcept)
      {
        auto p = boundry(e, n.get_index(), s_rcomparator);
        if (p.first == p.second)
          p.first->insert_root_node(&n);
        else
          insert_between(p.first, p.second, &n);
        return &n;
      }

      /**
       * @brief Insert @p n to a tree via @p e
       * @param e The entry node for the tree into which @p n will be inserted
       * @param n The node to be inserted
       * @returns pointer to @p n
       * @note User code is responsible to ensure @p e is already attached to
       * an rbtree or is the container node of an rbtree. If index of @n
       * duplicates others, @n will be put in a place in front of them
       */
      static pointer insert(reference e, reference n,
          index_policy::frontmost) noexcept(is_comparator_noexcept)
      {
        auto p = boundry(e, n.get_index(), s_comparator);
        if (p.first == p.second)
          p.first->insert_root_node(&n);
        else
          insert_between(p.first, p.second, &n);
        return &n;
      }

      /**
       * @brief Insert @p n to a tree via @p e
       * @param e The entry node for the tree into which @p n will be inserted
       * @param n The node to be inserted
       * @returns pointer to @p n
       * @note User code is responsible to ensure @p e is already inserted
       * into a rbtree or is the container node of an rbtree. @n will be
       * inserted once a suitable position is found without checkng index
       * duplication
       */
      static pointer
      insert(reference e, reference n, index_policy::nearest)
      noexcept(is_comparator_noexcept)
      {
        auto p = search(e, n.get_index());
        insert_between(p.first, p.second, &n);
        return &n;
      }

      /**
       * @brief Insert @p n to a tree via @p e
       * @param e The entry node for the tree into which @p n will be inserted
       * @param n The node to be inserted
       * @returns pointer to @p n if insertion was done successfully,
       * otherwise `nullptr`
       * @note User code is responsible to ensure @p e is already inserted
       * into a rbtree or it is the container node of an rbtree. If the index
       * of @n duplicates any node in that tree, this insertion will be
       * rejected
       */
      static pointer
      insert(reference e, reference n, index_policy::conflict)
      noexcept(is_comparator_noexcept)
      {
        auto p = search(e, n.get_index());
        insert_conflict(p.first, p.second, &n);
        return n.is_linked() ? &n : p.first;
      }

      /**
       * @brief Insert @p n to a tree via @p e
       * @param e The entry node for the tree into which @p n will be inserted
       * @param n The node to be inserted
       * @returns pointer to @p n
       * @note User code is responsible to ensure @p e is already inserted
       * into a rbtree or it is the container node of an rbtree. And each node
       * in that tree whose index is duplicated by @p n will be unlinked
       */
      static pointer
      insert(reference e, reference n, index_policy::unique)
        noexcept(is_comparator_noexcept)
      {
        auto l = lower_bound(e, n.get_index());
        auto u = upper_bound(*l, n.get_index());
        auto p = l->m_is_container ? l->m_r : l->prev();
        bool found = false;

        while (l != u)
        {
          auto x = l;
          l = l->next();
          if (x != &n)
            x->unlink();
          else
            found = true;
        }
        if (!found)
          insert_between(p, u, &n);
        return &n;
      }

      static comparator_type s_comparator;
      static struct rcomparator_type
      {
        rcomparator_type() = default;
        bool operator () (const Index &lhs, const Index &rhs) const
          noexcept(is_comparator_noexcept)
        { return !s_comparator(rhs, lhs); }
      } s_rcomparator;

      template<typename, typename, typename...>
      friend class rbtree_node;

      template<typename, typename, typename>
      friend class rbtree_iterator;

      template<typename, typename, typename>
      friend class rbtree_const_iterator;

      template<typename, typename, typename>
      friend class rbtree;


      node_pointer m_p;           /** < @brief Parent */
      node_pointer m_l;           /** < @brief Left child or predecessor */
      node_pointer m_r;           /** < @brief Right child */
      bool m_is_red;              /** < @brief Is red node */
      const bool m_is_container;  /** < @brief Is container node */
      bool m_has_l;               /** < @breef If this node has left chlid */
      bool m_has_r;               /** < @brief If this node has right child */
    };

    template<typename Index, typename Node, typename Tag>
    typename
    rbtree_node<Index, Node, rbtree_config<Tag>>::comparator_type
    rbtree_node<Index, Node, rbtree_config<Tag>>::s_comparator;

    template<typename Index, typename Node, typename Tag>
    typename
    rbtree_node<Index, Node, rbtree_config<Tag>>::rcomparator_type
    rbtree_node<Index, Node, rbtree_config<Tag>>::s_rcomparator;

    template<typename Index, typename Node, typename Tag>
    typename rbtree_node<Index, Node, rbtree_config<Tag>>::container_tag
    rbtree_node<Index, Node, rbtree_config<Tag>>::container;


	  /** For container */
    template<typename Index, typename Node, typename Tag>
    class rbtree_node<void, void, rbtree<Index, Node, Tag>>
		    : public rbtree_node<Index, Node, rbtree_config<Tag>>
    {
      template<typename, typename, typename...>
      friend class rbtree_node;
      template<typename, typename, typename>
      friend class rbtree;

    private:
	    rbtree_node() noexcept
			  : rbtree_node<Index, Node, rbtree_config<Tag>>(rbtree_node::container)
        , m_size(0)
	    {}

      rbtree_node(const rbtree_node &) = delete;

      rbtree_node(rbtree_node &&node) noexcept
        : rbtree_node<Index, Node, rbtree_config<Tag>>(std::move(node))
        , m_size(node.m_size)
      { node.m_size = 0; }

      rbtree_node &operator = (const rbtree_node &);

      rbtree_node &operator = (rbtree_node &&node) noexcept
      {
        if (&node != this)
        {
          this->~rbtree_node();
          new (this) rbtree_node(std::move(node));
        }
        return *this;
      }

      std::size_t m_size;
    };

    /**
	  * For single node
    */
    template<typename Index, typename Node, typename Tag>
    class rbtree_node<Index, Node, Tag>
      : public rbtree_node<Index, void>
      , public rbtree_node<Index, Node, rbtree_config<Tag>>
    {
      using detail = rbtree_node<void, void>;
      template<typename tag>
      using base_node = rbtree_node<Index, Node, rbtree_config<tag>>;

      template<typename Policy, typename Result = void>
      using insert_policy_sfinae
          = typename detail::insert_policy_sfinae<Policy, Result>;
    public:

      template<typename ...Arguments>
      rbtree_node(Arguments && ...arguments)
        noexcept(noexcept(Index(std::forward<Arguments>(arguments)...)))
        : rbtree_node<Index, void>(std::forward<Arguments>(arguments)...)
      {}


      const Index &get_index() const noexcept
      { return rbtree_node<Index, void>::m_index; }

      template<typename ...Arguments>
      void set_index(Arguments && ...arguments) noexcept
      {
        auto hint = base_node<Tag>::unlink_for_hint();
        rbtree_node<Index, void>::m_index
          = Index(std::forward<Arguments>(arguments)...);
        if (hint)
          base_node<Tag>::insert(*hint, *this,
              typename base_node<Tag>::config::default_insert_policy());
      }

      template<typename InsertPolicy, typename ...Arguments>
      insert_policy_sfinae<InsertPolicy>
      set_index_explicit(InsertPolicy policy, Arguments && ...arguments) noexcept
      {
        auto hint = base_node<Tag>::unlink_for_hint();
        rbtree_node<Index, void>::m_index
          = Index(std::forward<Arguments>(arguments)...);
        if (hint)
          base_node<Tag>::insert(*hint, *this, policy);
      }

      template<typename tag = Tag>
      typename std::enable_if<
        std::is_base_of<base_node<tag>, rbtree_node>::value,
        base_node<tag>>::type &get_node() noexcept
      { return *this; }
    private:

    };

	  /** For single const indexed node */
    template<typename Index, typename Node, typename Tag>
    class rbtree_node<const Index, Node, Tag>
      : public rbtree_node<Index, void>
      , public rbtree_node<Index, Node, rbtree_config<Tag>>
    {
      using detail = rbtree_node<void, void>;
      template<typename tag>
      using base_node = rbtree_node<Index, Node, rbtree_config<tag>>;

      template<typename Policy, typename Result = void>
      using insert_policy_sfinae
          = typename detail::insert_policy_sfinae<Policy, Result>;
    public:

      template<typename ...Arguments>
      rbtree_node(Arguments && ...arguments)
        noexcept(noexcept(Index(std::forward<Arguments>(arguments)...)))
        : rbtree_node<Index, void>(std::forward<Arguments>(arguments)...)
      {}

      template<typename tag = Tag>
      typename std::enable_if<
        std::is_base_of<base_node<tag>, rbtree_node>::value,
        base_node<tag>>::type &get_node() noexcept
      { return *this; }
    private:

    };


    template<typename Index, typename Node, typename ...Tag>
    class rbtree_node
      : public rbtree_node<Index, void>
      , public rbtree_node<Index, Node, rbtree_config<Tag>>...
    {

      using detail = rbtree_node<void, void>;
      template<typename tag>
      using base_node = rbtree_node<Index, Node, rbtree_config<tag>>;

      template<typename Policy, typename Result = void>
      using insert_policy_sfinae
          = typename detail::insert_policy_sfinae<Policy, Result>;

      template<typename tag>
      using tag_sfinae
          = typename std::enable_if<std::is_base_of<
              base_node<tag>, rbtree_node>::value, base_node<tag>>::type;

    public:

      template<typename ...Arguments>
      rbtree_node(Arguments && ...arguments)
        noexcept(noexcept(Index(std::forward<Arguments>(arguments)...)))
        : rbtree_node<Index, void>(std::forward<Arguments>(arguments)...)
      {}

      template<typename tag>
      typename std::enable_if<
        std::is_base_of<base_node<tag>, rbtree_node>::value,
        base_node<tag>>::type &
      get_node() noexcept
      { return *this; }

      template<typename ...Arguments>
      void set_index(Arguments && ...arguments) noexcept
      {
        auto helpers
          = std::make_tuple(
              set_index_helper<Tag>(*this, base_node<Tag>::unlink_for_hint())...);
        rbtree_node<Index, void>::m_index
          = Index(std::forward<Arguments>(arguments)...);
      }

      template<typename InsertPolicy, typename ...Arguments>
      insert_policy_sfinae<InsertPolicy>
      set_index(InsertPolicy policy, Arguments && ...arguments) noexcept
      {
        auto helpers
          = std::make_tuple(set_index_helper<Tag, decltype(policy)>(
                *this, base_node<Tag>::unlink_for_hint())...);
        rbtree_node<Index, void>::m_index
          = Index(std::forward<Arguments>(arguments)...);
      }


    private:

      template<typename tag,
        typename Policy = typename base_node<tag>::config::default_insert_policy>
      struct set_index_helper
      {
        rbtree_node &node;
        typename base_node<tag>::pointer hints;
        set_index_helper(rbtree_node &node, typename base_node<tag>::pointer hints)
          noexcept
          : node(node)
          , hints(hints)
        {  }

        set_index_helper(set_index_helper &&x) noexcept
          : node(x.node)
          , hints(x.hints)
        { x.hints = nullptr; }

        set_index_helper &operator = (set_index_helper &&x) noexcept
        {
          this->~set_index_helper();
          new (this) set_index_helper(std::move(x));
          return *this;
        }

        set_index_helper(const set_index_helper &) = delete;
        set_index_helper &operator = (const set_index_helper &) = delete;



        ~set_index_helper()
        {
          if (hints != nullptr)
            base_node<tag>::insert(*hints, node, Policy());
        }

      };

      template<typename tag, typename Policy>
      void reinsert(typename base_node<tag>::pointer hint, Policy policy) noexcept
      {
        reinsert(hint, *this, policy);
      }

      template<typename tag, typename Policy>
      void reinsert(typename base_node<tag>::pointer hint) noexcept
      {
        reinsert(hint, *this,
            typename base_node<tag>::config::default_insert_policy());
      }

    };

    template<typename Index, typename Node, typename ...Tag>
    class rbtree_node<const Index, Node, Tag...>
      : public rbtree_node<Index, void>
      , public rbtree_node<Index, Node, rbtree_config<Tag>>...
    {

      using detail = rbtree_node<void, void>;
      template<typename tag>
      using base_node = rbtree_node<Index, Node, rbtree_config<tag>>;

      template<typename Policy, typename Result = void>
      using insert_policy_sfinae
          = typename detail::insert_policy_sfinae<Policy, Result>;

      template<typename tag>
      using tag_sfinae
          = typename std::enable_if<std::is_base_of<
              base_node<tag>, rbtree_node>::value, base_node<tag>>::type;

    public:

      template<typename ...Arguments>
      rbtree_node(Arguments && ...arguments)
        noexcept(noexcept(Index(std::forward<Arguments>(arguments)...)))
        : rbtree_node<Index, void>(std::forward<Arguments>(arguments)...)
      {}

      template<typename tag>
      typename std::enable_if<
        std::is_base_of<base_node<tag>, rbtree_node>::value,
        base_node<tag>>::type &
      get_node() noexcept
      { return *this; }

    };

    /// \brief Quick access :)
    template<typename Index, typename Node>
    class rbtree_node<Index, Node> : public rbtree_node<Index, Node, Node>
    {
    public:
      template<typename ...Arguments>
      rbtree_node(Arguments && ...arguments)
        noexcept(noexcept(Index(std::forward<Arguments>(arguments)...)))
        : rbtree_node<Index, Node, Node>(
            std::forward<Arguments>(arguments)...)
      {}
    };

    /// \brief Quick access :)
    template<typename Index, typename Node>
    class rbtree_node<const Index, Node> : public rbtree_node<const Index, Node, Node>
    {
    public:
	    template<typename ...Arguments>
	    rbtree_node(Arguments && ...arguments)
	    noexcept(noexcept(Index(std::forward<Arguments>(arguments)...)))
			    : rbtree_node<Index, Node, Node>(
			    std::forward<Arguments>(arguments)...)
	    {}
    };
  }
}


#endif
