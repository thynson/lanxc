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

#include <lanxc/function.hpp>

#include <iostream>
#include <assert.h>

using namespace lanxc;

lanxc::function<void()> case1()
{
  long u = 3;
  long v = 1;
  long w = 0;
  auto lambda = [u,v](long x, long y, long z){
    assert(x == 1);
    assert(y == 2);
    assert(z == 3);
    std::cout << u << ' ' << x << ' ' << y << ' ' << z << std::endl;
  };
  function<void()> f;
  auto bind = std::bind(lambda, 1, 2, 3);
  std::cout << sizeof (bind) << std::endl;

  function<void()>(std::move(bind)).swap(f);
  return f;
}

int main()
{
  using namespace lanxc;
  int i = 0;
  function<int(int)> g =[&](int){ return i++; };
  g(0);
  assert(i == 1);
  case1()();

}