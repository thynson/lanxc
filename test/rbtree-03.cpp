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

constexpr int N = 100;
constexpr int M = 10;

class node : public rbtree_node<int, node>
{
public:
  node(int x)
      : rbtree_node(x)
  { }
};

std::mt19937 engine(static_cast<unsigned >(std::time(nullptr)));


int main()
{
  std::list<node> l;
  for (int i = 0; i < N; i++)
  {
    if (i % M == 0)
      cout << i/M << endl;

    l.emplace_back(i/M);
  }

  rbtree<int, node> t;

  for (auto &n : l)
    t.insert(n, index_policy::backmost());

  for (int i = 0; i < N/M; i++)
  {
    cout << "finding: " << i << endl;
    for (auto iter = t.cbegin(); iter != t.cend(); ++iter)
    {
      auto x = t.find(iter, i, index_policy::frontmost());
      assert (x != t.end());
      assert (x->get_index() == i);
    }
  }

  for (int i = 0; i < N/M; i++)
  {
    cout << "finding: " << i << endl;
    for (auto iter = t.cbegin(); iter != t.cend(); ++iter)
    {
      auto x = t.find(iter, i, index_policy::nearest());
      assert (x != t.end());
      assert (x->get_index() == i);
    }
  }

  for (int i = 0; i < N/M; i++)
  {
    cout << "finding: " << i << endl;
    for (auto iter = t.cbegin(); iter != t.cend(); ++iter)
    {
      auto x = t.find(iter, i, index_policy::backmost());
      assert (x != t.end());
      assert (x->get_index() == i);
    }
  }

}
