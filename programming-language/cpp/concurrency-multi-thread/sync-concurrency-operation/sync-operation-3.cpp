/*
同步并发操作

使用期望future等待一次性事件

在C++标准库中，有两种“期望”，使用两种类型模板实现，声明在头文件中: 
唯一期望(unique futures)(std::future<>)和共享期望(shared futures)(std::shared_future<>)。
std::future的实例只能与一个指定事件相关联，而std::shared_future的实例就能关联多个事件
*/



//带返回值的后台任务

//假设，有一个需要长时间的运算，需要其能计算出一个有效的值，但是现在并不迫切需要这个值
//可以启动一个新线程来执行这个计算，但是这就意味着你必须关注如何传回计算的结果，
//因为std::thread并不提供直接接收返回值的机制。这里就需要std::async函数模板

// 使用std::future从异步任务中获取返回值
#include <future>
#include <iostream>

int find_the_answer_to_ltuae();
void do_other_stuff();
int main()
{
  std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
  do_other_stuff();
  std::cout<<"The answer is "<<the_answer.get()<<std::endl;
}

//向异步任务传递参数
#include <string>
#include <future>
struct X
{
  void foo(int,std::string const&);
  std::string bar(std::string const&);
};
X x;
auto f1=std::async(&X::foo,&x,42,"hello");  // 调用p->foo(42, "hello")，p是指向x的指针
auto f2=std::async(&X::bar,x,"goodbye");  // 调用tmpx.bar("goodbye")， tmpx是x的拷贝副本
struct Y
{
  double operator()(double);
};
Y y;
auto f3=std::async(Y(),3.141);  // 调用tmpy(3.141)，tmpy通过Y的移动构造函数得到
auto f4=std::async(std::ref(y),2.718);  // 调用y(2.718)
X baz(X&);
std::async(baz,std::ref(x));  // 调用baz(x)
class move_only
{
public:
  move_only();
  move_only(move_only&&)
  move_only(move_only const&) = delete;
  move_only& operator=(move_only&&);
  move_only& operator=(move_only const&) = delete;

  void operator()();
};
auto f5=std::async(move_only());  // 调用tmp()，tmp是通过std::move(move_only())构造得到


auto f6=std::async(std::launch::async,Y(),1.2);  // 在新线程上执行
auto f7=std::async(std::launch::deferred,baz,std::ref(x));  // 在wait()或get()调用时执行
auto f8=std::async(
              std::launch::deferred | std::launch::async,
              baz,std::ref(x));  // 实现选择执行方式
auto f9=std::async(baz,std::ref(x));
f7.wait();  //  调用延迟函数