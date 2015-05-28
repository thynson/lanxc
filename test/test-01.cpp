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

#include <lanxc/intrus/rbtree.hpp>
#include <iostream>


struct tag;

namespace lanxc
{
  namespace intrus
  {
    template<>
      struct rbtree_config<tag> : rbtree_config<>
      {
        using default_insert_policy = lanxc::intrus::index_policy::conflict;

      };

  }
}


class X : public lanxc::intrus::rbtree_node<int, X, void, tag>
{
public:
  X(int x) : rbtree_node<int, X, void, tag>(x) {}

};


int main()
{
  lanxc::intrus::rbtree<int, X, void> t1;
  lanxc::intrus::rbtree<int, X, tag> t2;

  X x(1);
  t1.insert(x);

  for (auto &i : t1)
  {
    std::cout << i.get_node<void>().get_index() << std::endl;
  }
}
