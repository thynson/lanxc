

#include <iostream>
#include <lanxc/core/future.hpp>
#include <lanxc/link.hpp>
#include <list>
#include <set>

class mock_executor;

struct mock_deferred : lanxc::deferred , lanxc::link::list_node<mock_deferred>
{
  mock_executor &_mock_executor;
  lanxc::function<void()> routine;

  mock_deferred(mock_executor &me, lanxc::function<void()> f)
      : _mock_executor(me)
      , routine(std::move(f))
  { }

  void cancel() override
  {
    unlink();
  }

  ~mock_deferred() override
  {
    cancel();
  }


private:
  void execute() override
  {
    routine();
  }

};


class mock_executor : public lanxc::task_context
{
  friend class mock_deferred;

  lanxc::link::list<mock_deferred> deferred_list;
  std::set<std::weak_ptr<mock_deferred>> task_list;
public:


  std::shared_ptr<lanxc::deferred>
  defer(lanxc::function<void()> routine) override
  {
    std::shared_ptr<mock_deferred> p = std::make_shared<mock_deferred>(*this, std::move(routine));
    deferred_list.push_back(*p);
    return p;
  }

  std::shared_ptr<lanxc::alarm>
  schedule(std::uint64_t useconds, lanxc::function<void()> routine) override
  {
    return nullptr;
  }

  ~mock_executor() override = default;

protected:
  size_t process_tasks() override
  {
    lanxc::link::list<mock_deferred> l;
    size_t n = 0;
    deferred_list.swap(l);
    for (auto &i : l)
    {
      i.routine();
      n++;
    }
    return n;
  }

public:
  void run() override
  {
    do
    {
      process_tasks();
    }
    while(!deferred_list.empty());
  }
};


int main()
{
  mock_executor me;
  lanxc::future<int> f ([](lanxc::promise<int> p) {
    p.fulfill(0);
    std::cout << "!" << std::endl;
  });

  auto d = f
      .then([](int x)-> int
                  {
                    std::cout << "@" << std::endl;
                    return x+1;
                  })

   .then([](int x) -> lanxc::future<bool>
               {
                 std::cout << "!" << std::endl;
                 return lanxc::future<bool>([](lanxc::promise<bool> y)
                                            {
                                              std::cout << "!#" << std::endl;
                                              y.fulfill(true);
                                            }).then([](bool x) { return !x; });
               })
   .then([](bool x) -> lanxc::future<>
         {
           assert(!x);
           std::cout << "!" << std::endl;
           return lanxc::future<>(
               [](lanxc::promise<> x)
               {
//                 x.fulfill();
                 x.reject(int(0));
               }
           );
         })
   .caught<float>(
        [](float&)
        {
          assert(false);
        })
   .caught<int>(
        [](int &)
        {
          std::cout << "@@@1" << std::endl;
          return lanxc::future<>(
              [](lanxc::promise<> x)
              {
                x.reject(int(0));
              }
          );
        })
   .caught<float>(
       [](float)
       {
         assert(false);
       })
   .caught<int>(
       [](int)
       {
         std::cout << "@@@2" << std::endl;
       })
   .then(
       []()
       {
         std::cout << "###" << std::endl;
       }
   )


//   .then([](bool x) -> void
//         {
//           std::cout << "!" << std::endl;
//           return;
//         })
  .start(me);

  auto lambda = [](int) -> bool{ return true;};
  int a = 0;
  int &x = a;
  me.run();


}