/*
C++ 11的线程管理
*/

#include <thread>
#include <iostream>

static void do_some_work(){
    std::cout << "do some work" << std::endl;
}

class background_task
{
public:
  void operator()() const
  {
    do_something();
    do_something_else();
  }
};

int main()
{
    std::thread my_thread1(do_some_work);
    background_task f;
    std::thread my_thread2(f);
    std::thread my_thread3((background_task()));
    std::thread my_thread4{background_task()};
    std::thread my_thread5([]()->void{
        std::cout << "my thread 5" << std::endl;
    });

    my_thread1.join();// 等待线程结束，才能往下执行
    my_thread2.detach();//把my_thread2线程放入后台运行，不必等待线程结束，立即返回继续执行
    my_thread3.detach();
    my_thread4.detach();
    my_thread5.join();

    return 0;
}