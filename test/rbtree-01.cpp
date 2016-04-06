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

#include <lanxc/link/rbtree.hpp>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <random>
#include <cstdint>
#include <climits>
#include <ctime>

using namespace std;
using namespace lanxc::link;

class node : public rbtree_node<int, node>
{
public:
  node(int x)
      : rbtree_node(x)
  { }
};

std::mt19937 engine(static_cast<unsigned >(std::time(nullptr)));


void test_move()
{

  int random_number = engine();
  rbtree<int, node> t;
  node n(random_number);
  node m = std::move(n);
  assert(m.get_index() == random_number);
  t.insert(m);
  assert(&t.front() == &m);
  rbtree<int, node> r = std::move(t);
  assert(t.size() == 0);
  assert(r.size() == 1);
  assert(t.empty());
  assert(&r.front() == &m);
  n = std::move(m);
  assert(&r.front() == &n);

}

void test_swap()
{
  node a(1), b(2), c(3);
  rbtree<int, node> r, t;
  r.insert(a);
  t.insert(b);
  t.insert(c);
  t.swap(r);
  assert (t.size() == 1);
  assert (r.size() == 2);
}


void test_insert_policy()
{
  rbtree<int, node> t;
  int random_number = engine();
  node l(random_number), m(random_number), n(random_number);

  cout << "&l=" << &l << endl;
  cout << "&m=" << &m << endl;
  cout << "&n=" << &n << endl;

  t.clear();

  t.insert(l, index_policy::back());
  t.insert(m, index_policy::back());
  t.insert(n, index_policy::back());

  assert (t.size() == 3);
  assert (&t.front() == &l && &t.back() == &n);

  t.clear();

  t.insert(n, index_policy::front());
  t.insert(m, index_policy::front());
  t.insert(l, index_policy::front());

  assert (&t.front() == &l && &t.back() == &n);


  t.clear();

  t.insert(l, index_policy::nearest());
  t.insert(n, index_policy::nearest());
  t.insert(m, index_policy::nearest());

  assert(t.size() == 3);
  t.insert(l, index_policy::nearest());
  t.insert(n, index_policy::nearest());
  t.insert(m, index_policy::nearest());

  assert(t.size() == 3);

  t.insert(l, index_policy::unique());
  assert(l.is_linked());
  assert(!m.is_linked());
  assert(!n.is_linked());

  t.insert(m, index_policy::unique());
  assert(!l.is_linked());
  assert(m.is_linked());
  assert(!n.is_linked());

  t.insert(n, index_policy::unique());
  assert(!l.is_linked());
  assert(!m.is_linked());
  assert(n.is_linked());

  t.insert(l, index_policy::conflict());
  t.insert(m, index_policy::conflict());
  t.insert(n, index_policy::conflict());

  assert (t.size() == 1);
  assert (!l.is_linked());
  assert (!m.is_linked());
  assert (n.is_linked());

  t.insert(n, index_policy::conflict());
  assert (n.is_linked());

}

int main()
{
  test_move();
  test_swap();
  test_insert_policy();
}
