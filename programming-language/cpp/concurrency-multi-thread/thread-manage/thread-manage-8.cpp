/*
C++ 11的线程管理
转移线程所有权
std::thread支持移动语义，就意味着线程的所有权可以在函数外进行转移
*/

#include <thread>
#include <vector>
#include <string>
#include <iostream>

static void some_function()
{

}

static void some_other_function()
{

}

//函数返回std::thread对象
std::thread f()
{
  void some_function(){}
  return std::thread(some_function);
}

std::thread g()
{
  void some_other_function(int a){ }
  std::thread t(some_other_function,42);
  return t;
}

//当所有权可以在函数内部传递，就允许std::thread实例可作为参数进行传递
void f(std::thread t);
void g()
{
  void some_function();
  f(std::thread(some_function));
  std::thread t(some_function);
  f(std::move(t));
}

struct func
{
  int& i;
  func(int& i_) : i(i_) {}
  void operator() ()
  {
    for (unsigned j=0 ; j<1000000 ; ++j)
    {
      do_something(i);           
    }
  }
};

class scoped_thread
{
private:
    std::thread t;
public:
  explicit scoped_thread(std::thread t_):                 // 1
    t(std::move(t_))
  {
    if(!t.joinable())                                     // 2
      throw std::logic_error(“No thread”);
  }
  ~scoped_thread()
  {
    t.join();                                            // 3
  }
  scoped_thread(scoped_thread const&)=delete;
  scoped_thread& operator=(scoped_thread const&)=delete;
};

static void test_f()
{
    int some_local_state;
    scoped_thread t(std::thread(func(some_local_state)));    // 4
    do_something_in_current_main_thread();
    /*确保test_f函数退出之前，线程做出加入操作，不会遗忘*/
}

//量产线程，并等待它们结束
static void do_work(unsigned int i)
{
    std::cout << "thread " << i << std::endl;
}

static void test_f2()
{
    std::vector<std::thread> threads;
    for(unsigned i=0; i < 20; ++i)
    {
        threads.push_back(std::thread(do_work,i)); // 产生线程
    } 
    std::for_each(threads.begin(),threads.end(),
                  std::mem_fn(&std::thread::join)); // 对每个线程调用join()
}

int main()
{
    std::thread t1(some_function);            // t1 -> some_function
    std::thread t2=std::move(t1);            // t2 -> some_function, t1 -> null
    t1=std::thread(some_other_function);    // t1 -> some_other_function
    std::thread t3;                          // t3 -> null
    t3=std::move(t2);                        // t3 -> some_function , t2 -> null
    
    // 此时t1已经有关联的执行线程了，意图把t3的关联执行线程转交给t1，
    //所以系统直接调用std::terminate()终止程序继续运行
    t1=std::move(t3);

    test_f();
    test_f2();

    return 0;
}