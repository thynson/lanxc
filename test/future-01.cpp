#include <iostream>

#include <lanxc/future.hpp>

using namespace std;

int main()
{
  try
  {
    lanxc::thread_pool_scheduler scheduler;
    int i = 0;

    lanxc::future<int> f1(scheduler, [&i](lanxc::promise<int> p)
    {
      std::cout << "i=" << i++ << std::endl;
      p.set_result(1);
    });
    auto func = [&scheduler](int value)//->lanxc::future<>
    {
      std::cout << value << std::endl;
      return lanxc::future<>(scheduler, [](lanxc::promise<> p) {
        std::cout << "finished" << std::endl;
        p.set_result();
      });
    };
    auto f2 = f1.then(std::move(func));

    auto f3 = f2.then([&scheduler]()->lanxc::future<float, int>
    {
      return lanxc::future<float, int>(scheduler, [](lanxc::promise<float, int> p) {
        //p.set_result(42.0, 1024);
        std::cout << "going to throw exception" << std::endl;
        p.set_exception_ptr(std::make_exception_ptr(std::out_of_range("out of range")));
      });
    });


    auto f4 = f3.caught([&scheduler](std::exception_ptr e)
    {
      std::cout << "caught exception_ptr" << std::endl;
      try
      {
        std::rethrow_exception(e);
      } catch(std::exception &e) {
        std::cout << "okay: " << e.what() << std::endl;
      }
      return lanxc::future<>(scheduler, [](lanxc::promise<>p)
      {
        std::cout << "going to set result" << std::endl;
        p.set_result();
      });
    });

    auto f5 = f4.then([]()
    {
      std::cout << "okay" << std::endl;

    });

    f5.commit();
    scheduler.start();

  }
  catch(lanxc::invalid_future &i)
  {
    return 1;
  }
  return 0;
}
