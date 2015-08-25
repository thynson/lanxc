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

#ifndef LANXC_LINK_LIST_DEFINE_HPP_INCLUDED
#define LANXC_LINK_LIST_DEFINE_HPP_INCLUDED

/**
 * @defgroup intrusive_list Intrusive Bidirectional Linked List
 * @ingroup intrusive_data_structure
 */

namespace lanxc
{
  namespace link
  {

    template<typename Tag>
    struct list_config;

    template<typename Node, typename = void>
    class list_node;

    template<typename Node, typename = void>
    class list_iterator;

    template<typename Node, typename = void>
    class list_const_iterator;

    template<typename Node, typename = void>
    class list;


  }
}

#endif
