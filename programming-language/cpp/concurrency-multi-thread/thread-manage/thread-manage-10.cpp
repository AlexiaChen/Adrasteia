/*
C++ 11的线程管理
识别线程
线程标识类型是std::thread::id
*/

#include <thread>
#include <vector>
#include <iostream>

std::thread::id master_thread;
void some_core_part_of_algorithm()
{
  if(std::this_thread::get_id() == master_thread)
  {
    do_master_thread_work();
  }
  do_common_work();
}

int main()
{
    master_thread = get_master_thread_id();
    std::vector<std::thread> threads;
    for(int i = 0; i < 10; ++i) threads.push_back(std::thread(some_core_part_of_algorithm));
    for(auto t : threads) t.join();
    return 0;
}