/*
C++ 11的线程管理
后台线程运行
*/

#include <thread>
#include <iostream>
#include <cassert>

static void do_background_work()
{
    while(true)
    {
        //通常称分离线程为守护线程(daemon threads),UNIX中守护线程是指，且没有任何用户接口，并在后台运行的线程。
        do_some_daemon_work();
        msleep(3000);
    }
}

int main()
{
    std::thread t(do_background_work);
    t.detach();//线程t与主线程分离，就意味着主线程不能与之产生直接交互
    
    //被分离的线程t无法做加入操作，
    //换而言之，当std::thread对象使用t.joinable()返回的是true，就可以使用t.detach()。
    assert(!t.joinable());
    return 0;
}
