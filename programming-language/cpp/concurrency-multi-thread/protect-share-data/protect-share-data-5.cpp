/*
保护共享数据
进阶避免死锁的方案指导

1.避免嵌套锁
一个线程已获得一个锁时，再别去获取第二个。如果能坚持这个建议，因为每个线程只持有一个锁，锁上就不会产生死锁。
如果需要获取多个锁，使用一个std::lock来做这件事(对获取锁的操作/函数上锁)，避免产生死锁。

2.避免在持有锁时调用用户提供的代码
因为代码是用户提供的，你没有办法确定用户要做什么；用户程序可能做任何事情，包括获取锁。

3.使用固定顺序获取锁
当硬性条件要求你获取两个以上(包括两个)的锁，
并且不能使用std::lock单独操作来获取它们;那么最好在每个线程上，
用固定的顺序获取它们获取它们(锁)。

4.使用锁的层次结构
下面的示例代码

*/

#include <thread>
#include <mutex>

//层级锁的实现
class hierarchical_mutex
{
  std::mutex internal_mutex;

  unsigned long const hierarchy_value;
  unsigned long previous_hierarchy_value;

  // 使用了thread_local的值来代表当前线程的层级值,每个线程都有一个副本，被初始化为最大值
  static thread_local unsigned long this_thread_hierarchy_value;  

  void check_for_hierarchy_violation()
  {
    if(this_thread_hierarchy_value <= hierarchy_value)  // 如果锁的层级值大于或等于当前线程层级值就报错
    {
      throw std::logic_error(“mutex hierarchy violated”);
    }
  }

  void update_hierarchy_value()
  {
    previous_hierarchy_value=this_thread_hierarchy_value;  // 因为加了锁，所以保存之前的层级至
    this_thread_hierarchy_value=hierarchy_value; // 当前线程的层级值变为当前锁的层级至
  }

public:
  explicit hierarchical_mutex(unsigned long value):
      hierarchy_value(value),
      previous_hierarchy_value(0)
  {}

  void lock()
  {
    check_for_hierarchy_violation();
    internal_mutex.lock(); 
    update_hierarchy_value();  
  }

  void unlock()
  {
    this_thread_hierarchy_value = previous_hierarchy_value;  // 因为解锁了，恢复之前的层级值到当前线程层级值
    internal_mutex.unlock();
  }

  bool try_lock()
  {
    check_for_hierarchy_violation();
    if(!internal_mutex.try_lock())  // 不能持有锁时，不必更新层级值，直接返回
      return false;
    update_hierarchy_value();
    return true;
  }
};
thread_local unsigned long
     hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);  



//使用层次锁来避免死锁
hierarchical_mutex high_level_mutex(10000); // 层级值为10000
hierarchical_mutex low_level_mutex(5000);  // 层级值5000

int do_low_level_stuff();

int low_level_func()
{
  std::lock_guard<hierarchical_mutex> lk(low_level_mutex); //让low_level_mutex上锁
  return do_low_level_stuff();
}

void high_level_stuff(int some_param);

void high_level_func()
{
  std::lock_guard<hierarchical_mutex> lk(high_level_mutex); //让high_level_mutex上锁
  high_level_stuff(low_level_func()); // 为了获取high_level_stuff()的参数对互斥量上锁，之后调用low_level_func()
}

void thread_a()  // thread_a()遵守规则，所以它运行的没问题
{
  high_level_func();
}

hierarchical_mutex other_mutex(100); // 层级值100
void do_other_stuff();

void other_stuff()
{
  /*
  因此hierarchical_mutex将会产生一个错误，可能会是抛出一个异常，或直接终止程序。
  在层级互斥量上产生死锁，是不可能的，因为互斥量本身会严格遵循约定顺序，进行上锁。
  */
  high_level_func();  // 这个函数上了一个高层级锁10000，比other_mutex 100 大，违反了层级结构（thread_b这时层级比它高）
  do_other_stuff();
}

void thread_b() // thread_b()运行就不会顺利了
{
  std::lock_guard<hierarchical_mutex> lk(other_mutex); // 首先，它锁住了other_mutex
  other_stuff();
}

int main()
{
    std::thread t1(thread_a);
    std::thread t2(thread_b);
    t1.join();
    t2.join();
    return 0;
}