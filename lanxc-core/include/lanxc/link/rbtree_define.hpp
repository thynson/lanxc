/*
 * Copyright (C) 2015,2016 LAN Xingcan
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
/**
 *  @defgroup intrusive_rbtree Intrusive Red-Black Tree
 *  @ingroup intrusive_data_structure
 */

namespace lanxc
{
  namespace link
  {
    namespace index_policy
    {



      /**
       * @defgroup policy Indexing policy
       * @ingroup intrusive_rbtree
       * @{
       */

      /**
       * @brief Policy that deny a node being inserted into a tree if there
       * is equivalent node in it
       */
      struct conflict {};

      /**
       * @brief Policy that remove any equivalent node before a node is
       * inserted.
       */
      struct unique {};

      /**
       * @brief Pick the first node in equals range when doing lookup or
       * place node as front as possible when doing insert
       */
      struct front {};

      /**
       * @brief Pick the last node in equals range when doing lookup or
       * place node as back as possible when doing insert
       */
      struct back {};

      /**
       * @brief Pick the node found in first time when doing lookup; Or
       * place node as soon as possible once proper position was found,
       * in spite of the position of other equivalent nodes, when doing insert
       */
      struct nearest {};

      /** @} */


      /** @brief policy validator */
      template<typename InsertPolicy>
      struct is_insert_policy
      { static constexpr bool value = false; };

      template<typename LookupPolicy>
      struct is_lookup_policy
      { static constexpr bool value = false; };

      template<>
      struct is_insert_policy<conflict>
      { static constexpr bool value = true; };

      template<>
      struct is_insert_policy<unique>
      { static constexpr bool value = true; };

      template<>
      struct is_insert_policy<front>
      { static constexpr bool value = true; };

      template<>
      struct is_insert_policy<back>
      { static constexpr bool value = true; };

      template<>
      struct is_insert_policy<nearest>
      { static constexpr bool value = true; };

      template<>
      struct is_lookup_policy<front>
      { static constexpr bool value = true; };

      template<>
      struct is_lookup_policy<back>
      { static constexpr bool value = true; };

      template<>
      struct is_lookup_policy<nearest>
      { static constexpr bool value = true; };
    }

    template<typename Tag>
    class rbtree_config;

    template<typename Index, typename Node, typename ...Tags>
    class rbtree_node;

    template<typename Index, typename Node, typename Tag = void>
    class rbtree_iterator;

    template<typename Index, typename Node, typename Tag = void>
    class rbtree_const_iterator;

    template<typename Index, typename Node, typename Tag = void>
    class rbtree;


  }
}

