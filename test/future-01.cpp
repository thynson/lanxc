//
// Created by 蓝星灿 on 2017/7/17.
//

#include <lanxc/core/future.hpp>
#include <list>
#include <cassert>


class mock_executor : public lanxc::executor_context
{

  std::list<lanxc::function<void()>> routines;
public:

  void dispatch(lanxc::function<void()> routine) override
  {
    routines.push_back(std::move(routine));
  }

  void run() override
  {
    do
    {
      std::list<lanxc::function<void()>> l;
      routines.swap(l);
      for (auto &i : l) {
        i();
      }
    }
    while(!routines.empty());
  }
};

int main()
{
  mock_executor me;
  lanxc::future<int> f ([](lanxc::promise<int> p) {
    p.fulfill(0);

  });

  f.then([](int x)-> int { return x+1; })

   .then([](int x) -> lanxc::future<bool>
               {
                 assert(x == 1);
                 return lanxc::future<bool>([](lanxc::promise<bool> x)
                                            {
                                              x.fulfill(true);
                                            });
               })
   .then([](bool x) -> void
         {
           assert(x);
           return;
         })
   .start(me);
  me.run();


}