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

#include <lanxc/link.hpp>
#include <random>
#include <array>

struct X;
struct Y;

namespace lanxc
{
  namespace link
  {
    template<>
    class list_config<X> : public list_config<void>
    {
    public:
      static constexpr bool allow_constant_time_unlink = true;
    };
    template<>
    class list_config<Y> : public list_config<void>
    {
    public:
      static constexpr bool allow_constant_time_unlink = false;
    };
  }
}


struct X : public lanxc::link::list_node<X, X>
{
  unsigned int x;
  X(unsigned int x=0): x(x) {}
};

std::mt19937 engine(uint_fast32_t(time(nullptr)));

bool operator < (const X &lhs, const X &rhs)
{ return lhs.x < rhs.x; }

struct Y : public lanxc::link::list_node<Y, Y>
{

  unsigned int x;
  Y(unsigned int x=0): x(x) {}
};
bool operator < (const Y &lhs, const Y &rhs)
{ return lhs.x < rhs.x; }

static_assert(sizeof(lanxc::link::list<Y, Y>) >  4 * sizeof(void*), "");
static_assert(sizeof(lanxc::link::list<X, X>) == 4 * sizeof(void*), "");


template<typename T>
void test_list()
{

  std::array<T, 1000> x;
  lanxc::link::list<T, T> l;

  for (auto i = x.begin(); i != x.end(); ++i)
  {
    i->x = engine();
    l.push_back(*i);
  }

  l.sort();

  assert(std::is_sorted(l.begin(), l.end()));

}


int main()
{
  X x;
  Y y;
  lanxc::link::list<X, X> m;
  lanxc::link::list<Y, Y> n;
  m.push_back(x);
  n.push_back(y);
  assert(x.is_linked());

  test_list<X>();
  test_list<Y>();
  return 0;
}
