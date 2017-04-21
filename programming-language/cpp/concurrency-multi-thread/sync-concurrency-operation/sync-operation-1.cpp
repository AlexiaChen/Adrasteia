/*
同步并发操作

protect-share-data中有各种在线程间保护共享数据的方法。
当不仅想要保护数据，还想对单独的线程进行同步

可用于同步操作，形式上表现为条件变量(condition variables)和期望(futures)

等待一个事件或其他条件
*/

#include <mutex>
#include <chrono>
#include <condition_variable>
#include <queue>

//当一个线程等待另一个线程完成任务时
// 持续的检查共享数据标志(用于做保护工作的互斥量)，直到另一线程完成工作时对这个标志进行重设
bool flag;
std::mutex m;

void wait_for_flag()
{
  std::unique_lock<std::mutex> lk(m);
  while(!flag)
  {
    lk.unlock();  // 1 解锁互斥量
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 2 休眠100ms
    lk.lock();   // 3 再锁互斥量
  }
}

//以上的方法虽然可行，但是是对线程资源的一种浪费
// 使用C++ 标准提供的同步机制

//std::condition_variable一般是首选
//td::condition_variable_any更灵活通用，但是可能会对系统有隐性开销

//使用std::condition_variable处理数据等待
std::mutex mut;
std::queue<data_chunk> data_queue;  // 用来在两个线程之间传递数据的队列
std::condition_variable data_cond;

void data_preparation_thread()
{
  while(more_data_to_prepare())
  {
    data_chunk const data=prepare_data();
    std::lock_guard<std::mutex> lk(mut); // 对共享变量data_queue和data_cond访问，必须加锁
    data_queue.push(data);  // 写者把数据写入队列，给消费者消费
    data_cond.notify_one();  // 通知消费者数据已经写入完毕
    /*
    线程调用notify_one()通知条件变量时，处理数据的线程从睡眠状态中苏醒，重新获取互斥锁，并且对条件再次检查
    */
  }
}

void data_processing_thread()
{
  while(true)
  {
    std::unique_lock<std::mutex> lk(mut);  // 4
    // wait()会去检查lambda函数返回的条件，lambda返回true时，wait函数才返回，并继续持有锁。
    //lambda函数返回false，wait函数将解锁互斥量，并且将这个线程置于阻塞或等待状态。
    data_cond.wait(
         lk,[](){return !data_queue.empty();});  
    data_chunk data=data_queue.front();
    data_queue.pop();
    lk.unlock();  // 对共享变量data_queue和data_cond已访问完毕，立即解锁，降低锁的粒度
    process(data);
    if(is_last_chunk(data))
      break;
  }
}