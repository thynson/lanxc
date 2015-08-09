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
struct X;
struct Y;

namespace {
  template<>
  struct ::lanxc::link::list_config<X> : public list_config<void>
  {};

}
namespace lanxc
{
  namespace link
  {
    template<>
    struct list_config<X> : public list_config<void>
    {
      static constexpr bool allow_constant_time_unlink = true;
    };
    template<>
    struct list_config<Y> : public list_config<void>
    {
      static constexpr bool allow_constant_time_unlink = false;
    };
  }
}


struct X : public lanxc::link::list_node<X>
{

};

struct Y : public lanxc::link::list_node<Y>
{

};


static_assert(sizeof(lanxc::link::list<Y>) >  2 * sizeof(X), "");
static_assert(sizeof(lanxc::link::list<X>) == 2 * sizeof(X), "");


int main()
{
  X x;
  Y y;
  lanxc::link::list<X> m;
  lanxc::link::list<Y> n;
  m.push_back(x);
  n.push_back(y);
  assert(x.is_linked());
  return 0;
}
