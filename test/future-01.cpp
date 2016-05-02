#include <iostream>

#include <lanxc/future.hpp>

using namespace std;

int main()
{
  try
  {


    auto f = [](int value) { std::cout << value << std::endl; };
    int i = 0;

    lanxc::future<int> f1([&i](lanxc::promise<int> p)
    {
      std::cout << "i=" << i++ << std::endl;
      p.set_result(1);
    });
    auto func = [](int value)->lanxc::future<>
    {
      std::cout << value << std::endl;
      return lanxc::future<>([](lanxc::promise<> p) {
        p.set_result();
      });
    };
    auto f2 = f1.then(std::move(func));

    auto f3 = f2.then([]()->lanxc::future<float, int>
    {
      return lanxc::future<float, int>([](lanxc::promise<float, int> p) {
        //p.set_result(42.0, 1024);
        std::cout << "going to throw exception" << std::endl;
        p.set_exception_ptr(std::make_exception_ptr(std::out_of_range("out of range")));
      });
    });


    auto f4 = f3.caught([](std::exception_ptr e)
    {
      std::cout << "caught exception_ptr" << std::endl;
      try
      {
        std::rethrow_exception(e);
      } catch(std::exception &e) {
        std::cout << "okay: " << e.what() << std::endl;
      }
      return lanxc::future<>([](lanxc::promise<>p)
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

  }
  catch(lanxc::invalid_future &i)
  {
    return 1;
  }
  return 0;
}
