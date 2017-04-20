/*
保护共享数据
粗略的使用互斥量保护列表

某些情况下，使用全局变量没问题，但在大多数情况下，互斥量通常会与保护的数据放在同一个类中，而不是定义成全局变量。
这是面向对象设计的准则：将其放在一个类中，就可让他们联系在一起，也可对类的功能进行封装，并进行数据保护。
*/

#include <thread>
#include <mutex>
#include <algorithm>
#include <list>

std::list<int> some_list;  
std::mutex some_mutex;    // 全局互斥量保护some_list

//这两个函数中对数据的访问是互斥
void add_to_list(int new_value)
{
  std::lock_guard<std::mutex> guard(some_mutex);    
  some_list.push_back(new_value);
}

bool list_contains(int value_to_find)
{
  std::lock_guard<std::mutex> guard(some_mutex);    
  return std::find(some_list.begin(),some_list.end(),value_to_find) != some_list.end();
}
