/*
保护共享数据
保护共享数据的初始化过程

*/

#include <mutex>

//对于延迟初始化来说，一下代码在单线程下是安全的，
//但是在多线程下就不安全了，因为resource_ptr也是共享资源
std::shared_ptr<some_resource> resource_ptr;
void foo()
{
  if(!resource_ptr) // 没被初始化就初始化
  {
    resource_ptr.reset(new some_resource);  
  }
  resource_ptr->do_something();
}

//使用一个互斥量的延迟初始化(线程安全)过程
std::shared_ptr<some_resource> resource_ptr;
std::mutex resource_mutex;

void foo()
{
  std::unique_lock<std::mutex> lk(resource_mutex);  // 所有线程在此排队等待互斥量释放，虽然线程安全了，但是降低了效率
  if(!resource_ptr)
  {
    resource_ptr.reset(new some_resource);  // 只有初始化过程需要保护 
  }
  lk.unlock();
  resource_ptr->do_something();
}

//更好的办法是采用双重检查锁就不必排队死等了，但是会有隐患，非线程安全
void undefined_behaviour_with_double_checked_locking()
{
  if(!resource_ptr)  // 未被锁保护的读取操作
  {
    std::lock_guard<std::mutex> lk(resource_mutex);
    if(!resource_ptr)  // 2
    {
      resource_ptr.reset(new some_resource);  // 没有与其他线程里被锁保护的写入操作进行同步
    }
  }
  resource_ptr->do_something();  // 可能没有看到新创建的some_resource实例，然后调用do_something()后，得到不正确的结果
}

//使用std::call_once std::onece_flag来处理以上极端情况,完全线程安全
std::shared_ptr<some_resource> resource_ptr;
std::once_flag resource_flag;  // 1

void init_resource()
{
  resource_ptr.reset(new some_resource);
}

void foo()
{
  std::call_once(resource_flag,init_resource);  // 可以完整的进行一次初始化
  resource_ptr->do_something();
}


//使用std::call_once作为类成员的延迟初始化(线程安全)
class X
{
private:
  connection_info connection_details;
  connection_handle connection;
  std::once_flag connection_init_flag;

  void open_connection()
  {
    connection=connection_manager.open(connection_details);
  }
public:
  X(connection_info const& connection_details_):
      connection_details(connection_details_)
  {}
  void send_data(data_packet const& data)  
  {
    //第一个调用send_data()①或receive_data()的线程完成初始化过程。
    //使用成员函数open_connection()去初始化数据，也需要将this指针传进去。
    std::call_once(connection_init_flag,&X::open_connection,this);  
    connection.send_data(data);
  }
  data_packet receive_data()  
  {
    std::call_once(connection_init_flag,&X::open_connection,this);  
    return connection.receive_data();
  }
};


//在C++ 11标准前，这个不是线程安全的，但是C++ 11标准下，这么做是线程安全的
class my_class;
my_class& get_my_class_instance()
{
  static my_class instance;  // 线程安全的初始化过程
  return instance;
}