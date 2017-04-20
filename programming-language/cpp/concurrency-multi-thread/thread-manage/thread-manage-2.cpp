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
      do_something(i);           // 1. 潜在访问隐患：悬空引用
    }
  }
};

static void oops()
{
    int some_local_state=0;
    func my_func(some_local_state);
    std::thread my_thread(my_func);
    /// 与主线程分离，不等待线程结束
    my_thread.detach(); //oops函数返回了之后，可能my_thread才开始运行，局部变量some_local_state已经无效
}

int main()
{
    oops();
    return 0;
}