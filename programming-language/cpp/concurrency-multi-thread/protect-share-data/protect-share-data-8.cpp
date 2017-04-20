/*
保护共享数据
降低锁的粒度

*/

#include <mutex>

void get_and_process_data()
{
    std::unique_lock<std::mutex> my_lock(the_mutex);
    some_class data_to_process=get_next_data_chunk();//会访问到共享数据
    my_lock.unlock();  // 不要让锁住的互斥量越过process()函数的调用
    result_type result=process(data_to_process);//data_to_process是局部变量，可以不用加锁访问
    my_lock.lock(); // 2 为了写入数据，对互斥量再次上锁
    write_result(data_to_process,result);//会访问到共享数据
}