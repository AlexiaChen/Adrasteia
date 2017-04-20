/*
保护共享数据
死锁相关问题与解决方案

避免死锁的一般建议，就是让两个互斥量总以相同的顺序上锁：总在互斥量B之前锁住互斥量A，就永远不会死锁。

*/

#include <thread>
#include <mutex>


//交换操作中使用std::lock()和std::lock_guard
class some_big_object;
void swap(some_big_object& lhs,some_big_object& rhs);
class X
{
private:
  some_big_object some_detail;
  std::mutex m;
public:
  X(some_big_object const& sd):some_detail(sd){}

  friend void swap(X& lhs, X& rhs)
  {
    if(&lhs==&rhs) // 检查参数是否是不同的实例
      return;
    std::lock(lhs.m,rhs.m); // 同时锁住两个
    
    //提供std::adopt_lock参数除了表示std::lock_guard对象可获取锁之外，还将锁交由std::lock_guard对象管理，
    //而不需要std::lock_guard对象再去构建新的锁。
    std::lock_guard<std::mutex> lock_a(lhs.m,std::adopt_lock); 
    std::lock_guard<std::mutex> lock_b(rhs.m,std::adopt_lock); 
    swap(lhs.some_detail,rhs.some_detail);
  }
};
