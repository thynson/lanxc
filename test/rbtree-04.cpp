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


#include <lanxc/link/rbtree.hpp>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <random>
#include <cstdint>
#include <climits>
#include <ctime>
#include <list>

using namespace std;
using namespace lanxc::link;

class node : public rbtree_node<int, node, void, node>
{
public:
  node(int x = 0)
      : rbtree_node(x)
  { }

  friend bool operator < (const node &lhs, const node &rhs)
  { return lhs.get_index() < rhs.get_index(); }
};

int main()
{

  rbtree<int, node, void> tree;
  rbtree<int, node, node> tree2;

  node vn[100];
  std::mt19937 engine;
  for (node &x : vn)
  {
    tree.insert(x, index_policy::backmost());
    tree2.insert(x, index_policy::frontmost());
  }

  assert(std::is_sorted(tree.begin(), tree.end()));
  assert(std::is_sorted(tree2.begin(), tree2.end()));

  for (node &x : vn)
  {
    x.set_index_explicit(std::tuple<index_policy::frontmost, index_policy::backmost>(), engine());
  }
  assert(std::is_sorted(tree.begin(), tree.end()));
  assert(std::is_sorted(tree2.begin(), tree2.end()));
  tree.clear();
}
