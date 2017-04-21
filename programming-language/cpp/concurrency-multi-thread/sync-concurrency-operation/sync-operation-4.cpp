/*
同步并发操作

任务与期望

std::packaged_task<>对一个函数或可调用对象，绑定一个期望。
当std::packaged_task<> 对象被调用，它就会调用相关函数或可调用对象，
将期望状态置为就绪，返回值也会被存储为相关数据。
*/


//线程间传递任务
#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <utility>

std::mutex m;
std::deque<std::packaged_task<void()> > tasks;

bool gui_shutdown_message_received();
void get_and_process_gui_message();

void gui_thread()  // GUI线程
{
  while(!gui_shutdown_message_received())  // 只要不关闭GUI，就一直循环
  {
    get_and_process_gui_message();  // 进行轮询界面消息处理
    std::packaged_task<void()> task;
    {
      std::lock_guard<std::mutex> lk(m);
      if(tasks.empty())  // 任务队列为空继续循环
        continue;
      task=std::move(tasks.front());  // 把task队列的头的权限转移出来，有性能提升
      tasks.pop_front();// 移除队列头的任务
    }
    task();  // 执行取出的任务
  }
}

std::thread gui_bg_thread(gui_thread);

template<typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
  std::packaged_task<void()> task(f);  // 把任务函数打包为任务对象
  std::future<void> res=task.get_future();  // 得到将来值
  {
    std::lock_guard<std::mutex> lk(m);  // 对任务队列操作，加锁
    tasks.push_back(std::move(task));  
  }
  return res;
}

//promise & future简单用法
// promise 相当于一个异步provider, future相当于一个异步consumer
//  一个promise对象相当于你设置一个结果, 然后你就可以从与这个promise对象关联的future对象中获取这个结果了
#include <iostream>       // std::cout  
#include <functional>     // std::ref  
#include <thread>         // std::thread  
#include <future>         // std::promise, std::future  
void print_int (std::future<int>& fut) {  
    int x = fut.get();//当promise::set_value()设置了promise的共享状态值后，fut将会通过future::get()获得该共享状态值，若promise没有设置该值那么fut.get()将会阻塞线程直到共享状态值被promise设置  
    std::cout << "value: " << x << '\n';
}  
int main ()  
{  
    std::promise<int> prom;                      //创建一个promise对象  
    std::future<int> fut = prom.get_future();    //获取promise内部的future，fut将和promise共享promise中的共享状态，该共享状态用于返回计算结果  
    std::thread th1 (print_int, std::ref(fut));  //创建一个线程，并通过引用方式将fut传到print_int中  
    prom.set_value (10);                         //设置共享状态值  
                                                //  
    th1.join();//等待子线程  
    return 0;  
}  