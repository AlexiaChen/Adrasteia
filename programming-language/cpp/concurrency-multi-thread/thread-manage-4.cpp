/*
C++ 11的线程管理
使用C++ 的RAII机制来等待线程完成
*/

#include <thread>
#include <iostream>

class thread_guard
{
  std::thread& t;
public:
  explicit thread_guard(std::thread& t_):
    t(t_)
  {}
  ~thread_guard()
  {
    /*
    首先判断线程是否已加入，如果没有会调用join()②进行加入。
    这很重要，因为join()只能对给定的对象调用一次，
    所以对给已加入的线程再次进行加入操作时，将会导致错误。
    */
    if(t.joinable())
    {
      t.join();
    }
  }
  /*
  为了不让编译器自动生成它们。直接对一个对象进行拷贝或赋值是危险的，因为这可能会弄丢已经加入的线程。
  通过删除声明，任何尝试给thread_guard对象赋值的操作都会引发一个编译错误。
  */
  thread_guard(const thread_guard &) = delete;   // 3
  thread_guard& operator=(const thread_guard &) = delete;
};

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
    thread_guard g(t);
    do_something_in_current_main_thread();
    /*
    当线fn函数返回时，局部对象就要被逆序销毁了。
    因此，thread_guard对象g是第一个被销毁的，这时线程在析构函数中被加入到主线程中。
    即使do_something_in_current_main_thread抛出一个异常，这个销毁依旧会发生,对象g的析构函数还是会发生调用。
    */
}

int main()
{
    fn();
    return 0;
}