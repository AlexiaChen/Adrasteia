/*
保护共享数据
std::unique_lock更灵活的锁

*/

//交换操作中std::lock()和std::unique_lock的使用

#include <mutex>
#include <thread>

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
    if(&lhs==&rhs)
      return;
    std::unique_lock<std::mutex> lock_a(lhs.m,std::defer_lock); //std::defer_lock 先关联mutex对象，但暂时不对互斥量加锁，留到后面
    std::unique_lock<std::mutex> lock_b(rhs.m,std::defer_lock); 
    
    //因为std::unique_lock支持lock(), try_lock()和unlock()成员函数，
    //所以能将std::unique_lock对象传递到std::lock()
    std::lock(lock_a,lock_b); //  留到这里，斥量才在这里上锁
    swap(lhs.some_detail,rhs.some_detail);
  }
};

