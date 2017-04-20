/*
C++ 11的线程管理
*/

#include <thread>
#include <iostream>

struct func
{
  int& i;
  func(int& i_) : i(i_) {}
  void operator() ()
  {
    for (unsigned j=0 ; j<1000000 ; ++j)
    {
      do_something(i);           // 因为有主线程会等待，所以不会引起悬空引用
    }
  }
};

static void fn()
{
    int some_local_state=0;
    func my_func(some_local_state);
    std::thread t(my_func);
    
    try
    {
        do_something_in_current_main_thread();
    }
    catch(...)
    {
        t.join();  // 防止主线程遇到异常的时候，没对线程t做出分离或等待的决定
        throw;
    }
    t.join();  // 主线程正常运行并等待线程t
}

int main()
{
    fn();
    return 0;
}