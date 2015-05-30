/*
 * Copyright (C) 2013 LAN Xingcan
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
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>

using namespace std;
using namespace lanxc::intrus;

class X : public rbtree_node<int, X>
{
public:
  X(int x = 0)
      : rbtree_node(x)
  { }

  friend bool operator < (const X &lhs, const X &rhs)
  { return lhs.get_index() < rhs.get_index(); }

};


int main()
{

  rbtree<int, X> tree;
  X vn[100];
  std::mt19937 engine;
  for (X &x : vn)
    tree.insert(x, index_policy::backmost());

  for (X &x : vn)
  {
    x.set_index(engine());
  }
  assert(std::is_sorted(tree.begin(), tree.end()));
//
//  for (X &x : tree)
//    std::cout << x.get_index() << std::endl;
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();

  for (X &x : vn)
    tree.insert(x, index_policy::frontmost());
  for (X &x : vn)
    x.set_index(engine());
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();

  for (X &x : vn)
    tree.insert(x, index_policy::nearest());
  for (X &x : vn)
    x.set_index(engine());
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();

  for (X &x : vn)
    tree.insert(x, index_policy::unique());
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();

  for (X &x : vn)
    tree.insert(x, index_policy::replace());
  for (X &x : vn)
    x.set_index(engine());
  assert(std::is_sorted(tree.begin(), tree.end()));
  tree.clear();

}
