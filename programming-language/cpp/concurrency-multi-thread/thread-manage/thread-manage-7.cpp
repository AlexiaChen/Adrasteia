/*
C++ 11的线程管理
向线程函数传递参数
*/

#include <thread>
#include <string>
#include <iostream>
#include <memory>

static void fn(int i, const std::string& str)
{
    do_something(i,str);
}

static void oops(int some_param)
{
    char buffer[1024]; // buffer是局部变量
    sprintf(buffer, "%i",some_param);
    
    /*
    函数有很有可能会在字面值转化成std::string对象之前崩溃(oops)，从而导致一些未定义的行为。
    并且想要依赖隐式转换将字面值转换为函数期待的std::string对象，但因std::thread的构造函数会复制提供的变量，
    就只复制了没有转换成期望类型的字符串字面值。
    */
    std::thread t(fn,3,buffer); // 会有隐患
    t.detach();

    //在传递到std::thread构造函数之前就将字面值转化为std::string对象
    std::thread t2(fn, 3, std::string(buffer));//使用std::string，避免悬垂指针，解决隐患
    t2.detach();
}

static void update_data_for_widget(widget_id w,widget_data& data)
{
    do_something(w,data);
}

static void oops_again(widget_id w)
{
    widget_data data;
    /*
    虽然update_data_for_widget的第二个参数期待传入一个引用，但是std::thread的构造函数并不知晓；
    std::thread构造函数无视update_data_for_widget函数期待的参数类型，并盲目的拷贝已提供的变量。
    当线程调用update_data_for_widget函数时，传递给函数的参数是data变量在std:thread构造函数内部拷贝的引用，而非数据本身的引用。
    */
    std::thread t(update_data_for_widget,w,data); 
    display_status();
    t.join();
    
    /*
    当线程结束时，std::thread构造函数内部拷贝数据将会在数据更新阶段被销毁，且process_widget_data将会接收到没有修改的data变量
    */
    process_widget_data(data); 

    
    // 使用std::ref将参数转换成引用的形式
    // 在这之后，update_data_for_widget就会接收到一个data变量的引用，而非一个data变量拷贝的引用
    std::thread t2(update_data_for_widget,w,std::ref(data));
    display_status();
    t2.join();
    process_widget_data(data);  //接受的data变量是更新后的数据
}

class X
{
public:
  void do_lengthy_work(int num) {use_param(num);}
};

static void member_function_thread()
{
    X my_x;
    int num = 5;
    std::thread t(&X::do_lengthy_work,&my_x,num);
    t.join(); // 不能detach 
}

static void process_big_object(std::unique_ptr<big_object> p)
{
    int data = p->get_data();
    do_something(data);
}

static void move_thread()
{
    std::unique_ptr<big_object> p = std::make_unique<big_object>();
    p->prepare_data(42);

    /*
    在std::thread的构造函数中指定std::move(p),
    big_object对象的所有权就被首先转移到新创建线程的的内部存储中，
    之后传递给process_big_object函数。
    */
    std::thread t(process_big_object,std::move(p));
    t.detach();
}

int main()
{
    std::thread t1(fn,3,"hello");
    t1.join();
    oops(6);
    oops_again(0xff);
    member_function_thread();
    return 0;
}