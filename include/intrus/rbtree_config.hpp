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

#ifndef LANXC_INTRUS_RBTREE_CONFIG_HPP_INCLUDED
#define LANXC_INTRUS_RBTREE_CONFIG_HPP_INCLUDED

#include "../functional.hpp"
#include "rbtree_define.hpp"

#include <type_traits>


namespace lanxc
{
  namespace intrus
  {

    template<>
    struct rbtree_config<void>
    {
      /** @brief Comparator adapter, must meets requiement of strict weak
       * ordering binary predicate */
      template<typename T> using comparator = less<T>;

      /**
       * @brief Pointer adapter
       * @note This is only used to link nodes among the tree, won't change
       *       API of rbtree or its iterator
       * Override it to alias to other pointer template, to link node in
       * different way for your need, e.g., alias to boost::offset_ptr when
       * you want another process access this tree via shared memory
       */
      template<typename T> using node_pointer = T*;

      /**
       * @brief Default policy for lookup
       */
      using default_lookup_policy = index_policy::nearest;

      /** @brief Default policy for insert */
      using default_insert_policy = index_policy::unique;

      /**
       * @brief Allow a node be removed from a tree in constant time
       * @note Enable this will make an rbtree costs linear time to count
       * the number of its node
       */
      constexpr static bool allow_constant_time_unlink = true;

      /** @brief Store predecessor and successor of one node in its child link
       * when they are free to speedup iteration, but should slowdown
       * insertion and deletion */
      constexpr static bool optimize_for_iteration = true;

      /**
       * @brief set this to true to make methods set_index, get_index, unlink,
       * is_linked and get_node be public member of rbtree_node, otherwise
       * they are protected member.
       */
      constexpr static bool public_node_api = true;
    };

    template<typename Tag>
    struct rbtree_config : public rbtree_config<>
    { };

  }
}

#endif
