/*
保护共享数据
不同域中互斥量所有权的传递

*/

//函数get_lock()锁住了互斥量，然后准备数据，返回锁的调用函数
std::unique_lock<std::mutex> get_lock()
{
  extern std::mutex some_mutex;
  std::unique_lock<std::mutex> lk(some_mutex);
  prepare_data();
  return lk;  // 自动调用了unique_lock的移动构造函数，无需担心性能问题
}
void process_data()
{
  std::unique_lock<std::mutex> lk(get_lock());  // 把所有权转移到了process_data的范围内了
  do_something();
}