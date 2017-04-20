/*
保护共享数据
精心组织代码来保护共享数据

切勿将受保护数据的指针或引用传递到互斥锁作用域之外，无论是函数返回值，
还是存储在外部可见内存，亦或是以参数的形式传递到用户提供的函数中去。
*/

#include <thread>
#include <mutex>

class some_data
{
  int a;
  std::string b;
public:
  void do_something();
};

class data_wrapper
{
private:
  some_data data;
  std::mutex m;
public:
  template<typename Function>
  void process_data(Function func)
  {
    std::lock_guard<std::mutex> lock(m);
    func(data);    // 1 传递“保护”数据给用户函数,用户就可以在互斥锁作用域外修改了
  }
};

some_data* unprotected;

void malicious_function(some_data& protected_data)
{
  unprotected = &protected_data;
}

data_wrapper x;
void foo()
{
  x.process_data(malicious_function);    //传递一个恶意函数,让它指向受保护数据
  unprotected->do_something();    //在无保护的情况下访问保护数据
}

int main()
{
    foo();
    return 0;
}


