/*
C++ 内存模型和原子操作

对象，内存位置和并发

为了避免条件竞争，两个线程就需要一定的执行顺序
1.使用互斥量来确定访问的顺序
2.原子操作(atmic operations)同步机制，决定两个线程的访问顺序

如果不去规定两个不同线程对同一内存地址访问的顺序，那么访问就不是原子的
*/


//使用std::atomic_flag实现自旋互斥锁
//足够std::lock_guard<>使用
class spinlock_mutex
{
  std::atomic_flag flag;
public:
  spinlock_mutex():
    flag(ATOMIC_FLAG_INIT)
  {}
  void lock()
  {
    while(flag.test_and_set(std::memory_order_acquire));
  }
  void unlock()
  {
    flag.clear(std::memory_order_release);
  }
};

//std::atomic的相关操作
std::atomic<bool> b;
bool x=b.load(std::memory_order_acquire);
b.store(true);
x=b.exchange(false, std::memory_order_acq_rel);

bool expected=false;
extern atomic<bool> b; 
while(!b.compare_exchange_weak(expected,true) && !expected);

std::atomic<bool> b;
bool expected;
b.compare_exchange_weak(expected,true,
  memory_order_acq_rel,memory_order_acquire);
b.compare_exchange_weak(expected,true,memory_order_acq_rel);


//std::atomic:指针运算
class Foo{};
Foo some_array[5];
std::atomic<Foo*> p(some_array);
Foo* x=p.fetch_add(2);  // p加2，并返回原始值
assert(x==some_array);
assert(p.load()==&some_array[2]);
x=(p-=1);  // p减1，并返回原始值
assert(x==&some_array[1]);
assert(p.load()==&some_array[1]);