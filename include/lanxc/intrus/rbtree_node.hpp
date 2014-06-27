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
  namespace intrus
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
    class rbtree_node<Index, Node, Tag, rbtree_node<void, void>>
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

      rbtree_node(container_tag) noexcept
        : m_p(this), m_l(this), m_r(this)
        , m_is_red(true), m_is_container(true)
        , m_has_l(false), m_has_r(false)
      {}

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
        this->~rbtree_node();
        new (this) rbtree_node(std::move(n));
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
        return unlink_and_get_next() != nullptr;
      }


    private:

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
        pointer p = this;
        while (p->m_p->m_p != this)
        {
          if (p->m_p->m_p == p)
            return nullptr;
          else
            p = p->m_p;
        }
        return p;
      }

      /** @brief Get root node */
      pointer get_root_node() noexcept
      {
        pointer p = this;
        while (p->m_p->m_p != this)
        {
          if (p->m_p->m_p == p)
            return nullptr;
          else
            p = p->m_p;
        }
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
        if (m_has_r) return m_r->front();
        else return m_r;
      }

      pointer prev() noexcept
      {
        if (m_has_l) return m_l->back();
        else return m_l;
      }


      const_pointer next() const noexcept
      {
        if (m_has_r) return m_r->front();
        else return m_r;
      }

      const_pointer prev() const noexcept
      {
        if (m_has_l) return m_l->back();
        else return m_l;
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


      /**
       * @brief Insert a node as left child of this node
       * @note User is responsible for ensure this node does not have left
       * child
       */
      void insert_to_left(pointer node) noexcept
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
      void insert_to_right(pointer node) noexcept
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
      }

      /** @brief Insert a node as predecessor of this node */
      void insert_before (pointer node) noexcept
      {
        node->unlink();

        if (m_is_container)
          insert_root_node(node);
        else if (m_has_l)
          prev()->insert_to_left(node);
        else
          insert_to_left(node);
      }

      /** @brief Insert a node as successor of this node */
      void insert_after(pointer node) noexcept
      {
        node->unlink();

        if (m_is_container)
          insert_root_node(node);
        else if (this->m_has_r)
          next()->insert_to_right(node);
        else
          insert_to_right(node);
      }

      void unlink_cleanup() noexcept
      {
        m_l = m_r = m_p = nullptr;
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
        return false;
      }

      pointer unlink_and_get_next() noexcept
      {
        if (!is_linked())
          return nullptr;
        pointer y = next(), x;

        if (m_has_l && m_has_r)
          swap_nodes(*this, *y);

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
          rebalance_for_unlink(x);
        unlink_cleanup();
        return y;

      }

      /**
       * @brief Insert node as parent's chlid, left child or right chlid are
       * all possible.
       * @param parent The node want child
       * @param node The node want to be inserted
       */
      static void insert(pointer entry, pointer node) noexcept
      {
        if (entry->m_is_container)
          entry->insert_root_node(node);
        else if (!entry->m_has_l)
          entry->insert_to_left(node);
        else if (!entry->m_has_r)
          entry->insert_to_right(node);
        else
          entry->next()->insert_to_left(node);
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
        if (prev == next)
          insert(prev, node);
        else if (prev->m_is_container)
          next->insert_to_left(node);
        else if (next->m_is_container)
          prev->insert_to_right(node);
        else if (prev->m_has_r)
          next->insert_to_left(node);
        else
          prev->insert_to_right(node);
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

      /**
       * @brief Insert a node between prev and next and another node with same index
       * will be overrided (unlinked from tree)
       * @param prev The node will become predecessor of the node after insertion
       * is done
       * @param next The node will be successor of the node after insertion is
       * done
       * @param node The node want to be inserted
       * @note User code should ensure that prev is predecessor of #next
       */
      static void insert_replace(pointer prev,
          pointer next, pointer node) noexcept
      {
        if (prev == next)
        {
          if (prev->m_is_container)
            prev->insert_root_node(node);
          else
          {
            auto *l = prev->prev(), *r = prev->next();
            prev->unlink();
            insert_between(l, r, node);
          }
        }
        else
          insert_between(prev, next, node);
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
      }

      /** @brief Rebalance the tree after unlink */
      static void
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
              // as node is black but w is red, the following assertion must
              // satisfied
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
              // as node is black but w is red, the following assertion must
              // satisfied
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
          return;
        else if (!node->is_root_node())
          node = node->get_root_node();
        if (node)
          node->m_is_red = false;
      }

      /**
       * @brief Swap two nodes in the tree
       * @note This will generally break the order or nodes, so it's declared
       * privately
       */
      static void swap_nodes(reference lhs, reference rhs) noexcept
      {
        rbtree_node tmp;
        move(lhs, tmp);
        move(rhs, lhs);
        move(tmp, rhs);
      }

      static void move(reference dst, reference src) noexcept
      {

        if (src.is_linked())
        {
          if (src.m_is_container)
          {
            dst.~rbtree_node();
            new (&dst) rbtree_node(container);
            if (src.is_empty_container_node())
            {
              src.unlink_cleanup();
              return;
            }
          }

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


      /**
       * @brief Search the position where specified index is suitable to be
       * @param entry A node in the tree which we want to search in
       * @param index The index we want to search for
       * @returns a pair of pointer to rbtree_node, if the pair is the same,
       * then the index is equals to the index of node which the two pointer
       * point to, else if the pair is not the same, the index is greater than
       * the first of the pair and less than the second of the pair
       */
      template<typename Reference>
      static std::pair<pointer, pointer>
      search(Reference &entry, const Index &index) noexcept
      {
        auto *p = &entry;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
            return std::make_pair(p, p);
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmp = [&] (reference node) noexcept -> bool
        { return s_comparator(node.internal_get_index(), index); };

        auto rcmp = [&] (reference node) noexcept -> bool
        { return !s_comparator(index, node.internal_get_index()); };

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
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_r->m_is_container)
            {
              auto x = cmp(*p->m_r);
              auto y = rcmp(*p->m_r);
              if (x != y)
                return std::make_pair(p->m_r, p->m_r);
              if (x)
                p = p->m_r;
              else
                break;
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
              else if (x)
              {
                p = p->m_p;
                continue;
              }
            }

            if (!p->m_l->m_is_container)
            {
              auto x = cmp(*p->m_l);
              auto y = rcmp(*p->m_l);
              if (x != y)
                return std::make_pair(p->m_l, p->m_l);
              if (x)
                p = p->m_l;
              else
                break;
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
       * @brief Get the boundry of a rbtree for specified index
       * @param entry The entry node of tree
       * @param index The specified index depends on which the boundry is find
       */
      template<typename Reference, typename Comparator>
      static auto
      boundry(Reference &entry, const Index &index, const Comparator &comp)
        noexcept -> std::pair<typename std::add_pointer<Reference>::type,
            typename std::add_pointer<Reference>::type>
      {
        auto *p = &entry;

        if (p->m_is_container)
        {
          if (p->is_empty_container_node())
            return std::make_pair(p, p);
          else
            p = p->get_root_node_from_container_node();
        }

        auto cmper = [&index, &comp] (Reference &node) noexcept -> bool
        { return comp(node.internal_get_index(), index); };

        bool hint_result = cmper(*p);

        if (hint_result)
        {
          while (!p->m_p->m_is_container)
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
          while (!p->m_p->m_is_container)
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

      static pointer find(reference entry,
          const Index &index, index_policy::backmost) noexcept(is_comparator_noexcept)
      {
        auto *p = boundry(entry, index, s_rcomparator).first;

        if (s_comparator(index_fetcher(*p), index) != s_rcomparator(index_fetcher(*p), index))
          return p;
        else
          return nullptr;
      }

      static pointer find(reference entry,
          const Index &index, index_policy::frontmost) noexcept(is_comparator_noexcept)
      {
        auto *p = boundry(entry, index).second;

        if (cmper(index_fetcher(*p), index) != !cmper(index, index_fetcher(*p)))
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
      {
        return boundry(entry, index, s_rcomparator).second;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to
       * a tree; and if duplicate is permitted, the node is insert after any
       * node duplicate with this node
       */
      static pointer insert(reference entry, reference node,
          index_policy::backmost) noexcept(is_comparator_noexcept)
      {
        auto p = boundry(entry, node.internal_get_index(), s_rcomparator);
        insert_between(p.first, p.second, &node);
        return &node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param entry The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note Use is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert before any
       * node duplicate with this node
       */
      static pointer insert(reference entry, reference node,
          index_policy::frontmost) noexcept(is_comparator_noexcept)
      {
        auto p = boundry(entry, node.internal_get_index(), s_comparator);
        insert_between(p.first, p.second, &node);
        return &node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param entry The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note Use is responsible to ensure hint_node is already attached to a
       * tree; and if duplicate is permitted, the node is insert with least
       * searching being done
       */
      static pointer insert(reference entry, reference node,
          index_policy::nearest) noexcept(is_comparator_noexcept)
      {
        auto p = search(entry, node.internal_get_index());
        insert_between(p.first, p.second, &node);
        return &node;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to
       * a tree; and duplicated node is not allow
       */
      static pointer insert(reference entry, reference node,
          index_policy::conflict) noexcept(is_comparator_noexcept)
      {
        auto p = search(entry, node.internal_get_index());
        insert_conflict(p.first, p.second, &node);
        return node.is_linked() ? &node : p.first;
      }

      /**
       * @brief Insert a node to a tree that hint_node is attached to
       * @param hint_node The node which is attached into a rbtree for hinting
       * where node should be placed to
       * @param node The node to be insert
       * @note User is responsible to ensure hint_node is already attached to
       * a tree; and duplicated node will be replaced by this node
       */
      static pointer insert(reference entry, reference node,
          index_policy::unique) noexcept(is_comparator_noexcept)
      {
        auto l = lower_bound(entry, node.internal_get_index());
        auto u = upper_bound(*l, node.internal_get_index());
        auto p = l->prev();

        while (l != u)
        {
          auto x = l;
          l = l->next();
          x->unlink();
        }
        insert_between(p, u, &node);
        return &node;
      }

      static pointer insert(reference entry, reference node,
          index_policy::replace) noexcept(is_comparator_noexcept)
      {
        auto p = search(entry, node.internal_get_index());
        insert_replace(p.first, p.second, &node);
        return node.is_linked() ? &node : p.first;
      }

      static pointer insert(reference entry, reference node,
          index_policy::replace_frontmost) noexcept(is_comparator_noexcept)
      {
        auto p = boundry(entry, node.internal_get_index());
        if (s_comparator(index_fetcher(p.second), node)
            != s_rcomparator(index_fetcher(p.second), node))
          insert_replace(p.first, p.second, &node);
        else
          insert_between(p.first, p.second, &node);
        return &node;
      }

      static pointer insert(reference entry, reference node,
          index_policy::replace_backmost) noexcept(is_comparator_noexcept)
      {
        auto p = boundry(entry, node.internal_get_index(), s_rcomparator);
        if (s_comparator(index_fetcher(p.first), node)
            != s_rcomparator(index_fetcher(p.first), node))
          insert_replace(p.first, p.second, &node);
        else
          insert_between(p.first, p.second, &node);
        return &node;
      }

      const Index &internal_get_index() const
      {
        const Node &n = static_cast<const Node &>(*this);
        return n.rbtree_node<Index, void>::m_index;
      }

      Index &internal_get_index()
      {
        Node &n = static_cast<Node&>(*this);
        return n.rbtree_node<Index, void>::m_index;
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


      node_pointer m_p;
      node_pointer m_l;
      node_pointer m_r;
      bool m_is_red;
      const bool m_is_container;
      bool m_has_l;
      bool m_has_r;

    };

    template<typename Index, typename Node, typename Tag>
    typename
    rbtree_node<Index, Node, Tag, rbtree_node<void, void>>::comparator_type
    rbtree_node<Index, Node, Tag, rbtree_node<void, void>>::s_comparator;

    template<typename Index, typename Node, typename Tag>
    typename
    rbtree_node<Index, Node, Tag, rbtree_node<void, void>>::rcomparator_type
    rbtree_node<Index, Node, Tag, rbtree_node<void, void>>::s_rcomparator;


    template<typename Index, typename Node, typename Tag>
    class rbtree_node<Index, Node, Tag>
      : public rbtree_node<Index, void>
      , public rbtree_node<Index, Node, Tag, rbtree_node<void, void>>
    {
      using detail = rbtree_node<void, void>;
      template<typename tag>
      using base_node = rbtree_node<Index, Node, tag, rbtree_node<void, void>>;

      template<typename Policy, typename Result = void>
      using insert_policy_sfinae
          = typename detail::insert_policy_sfinae<Policy, Result>;
    public:

      template<typename ...Arguments>
      rbtree_node(Arguments && ...arguments)
        noexcept(noexcept(Index(std::forward<Arguments>(arguments)...)))
        : rbtree_node<Index, void>(std::forward<Arguments>(arguments)...)
      {}


      const Index &get_index() const
      { return rbtree_node<Index, void>::m_index; }

      template<typename ...Arguments>
      void set_index(Arguments && ...arguments)
      {
        auto hint = base_node<Tag>::unlink_and_get_next();
        rbtree_node<Index, void>::m_index
          = Index(std::forward<Arguments>(arguments)...);
        if (hint)
          base_node<Tag>::insert(*hint, *this,
              typename base_node<Tag>::config::default_insert_policy());
      }

      template<typename InsertPolicy, typename ...Arguments>
      insert_policy_sfinae<InsertPolicy>
      set_index(index_policy::conflict policy, Arguments && ...arguments)
      {
        auto hint = base_node<Tag>::unlink_and_get_next();
        rbtree_node<Index, void>::m_index
          = Index(std::forward<Arguments>(arguments)...);
        base_node<Tag>::insert(*hint, *this, policy);
      }

      template<typename config = Tag>
      typename std::enable_if<
        std::is_base_of<base_node<config>, rbtree_node>::value,
        base_node<config>>::type &get_node()
      { return *this; }
    private:

    };

    template<typename Index, typename Node, typename ...Tag>
    class rbtree_node
      : public rbtree_node<Index, void>
      , public rbtree_node<Index, Node, Tag, rbtree_node<void, void>>...
    {

      using detail = rbtree_node<void, void>;
      template<typename tag>
      using base_node = rbtree_node<Index, Node, tag, rbtree_node<void, void>>;

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

      const Index &get_index() const
      { return rbtree_node<Index, void>::m_index; }

      template<typename tag>
      typename std::enable_if<
        std::is_base_of<base_node<tag>, rbtree_node>::value,
        base_node<tag>>::type &
      get_node()
      { return *this; }

      template<typename ...Arguments>
      void set_index(Arguments && ...arguments) noexcept
      {
        std::tuple<set_index_helper<Tag>...> helpers
          = std::make_tuple(
              set_index_helper<Tag>(*this, base_node<Tag>::unlink_and_get_next())...);
        rbtree_node<Index, void>::m_index
          = Index(std::forward<Arguments>(arguments)...);
      }

      template<typename InsertPolicy, typename ...Arguments>
      insert_policy_sfinae<InsertPolicy>
      set_index(InsertPolicy policy, Arguments && ...arguments) noexcept
      {
        std::tuple<set_index_helper<Tag, decltype(policy)>...> helpers
          = std::make_tuple(set_index_helper<Tag, decltype(policy)>(
                *this, base_node<Tag>::unlink_and_get_next())...);
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
      void reinsert(typename base_node<tag>::pointer hint, Policy policy)
      {
        reinsert(hint, *this, policy);
      }

      template<typename tag, typename Policy>
      void reinsert(typename base_node<tag>::pointer hint)
      {
        reinsert(hint, *this,
            typename base_node<tag>::config::default_insert_policy());
      }

    };

    /// \brief Quick access :)
    template<typename Index, typename Node>
    class rbtree_node<Index, Node> : public rbtree_node<Index, Node, void>
    {
    public:
      template<typename ...Arguments>
      rbtree_node(Arguments && ...arguments)
        noexcept(noexcept(Index(std::forward<Arguments>(arguments)...)))
        : rbtree_node<Index, Node, rbtree_config<>>(
            std::forward<Arguments>(arguments)...)
      {}
    };
  }
}


#endif
