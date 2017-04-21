/*
同步并发操作

使用std::promise
*/

//使用“promise”解决单线程多连接问题
#include <future>

void process_connections(connection_set& connections)
{
  while(!done(connections))
  {
    for(connection_iterator  // 每一次循环，程序都会依次的检查每一个连
            connection=connections.begin(),end=connections.end();
          connection!=end;
          ++connection)
    {
      if(connection->has_incoming_data())  // 在接受队列中是否有数据
      {
        data_packet data=connection->incoming();
        std::promise<payload_type>& p=
            connection->get_promise(data.id);  
        p.set_value(data.payload);
      }
      if(connection->has_outgoing_data())  //正在发送已入队的传出数据
      {
        outgoing_packet data=
            connection->top_of_outgoing_queue();
        connection->send(data.payload);
        data.promise.set_value(true);  // 传出数据相关的“承诺”将置为true，来表明传输成功
      }
    }
  }
}


//为“期望”存储“异常”
double square_root(double x)
{
  if(x<0)
  {
    throw std::out_of_range(“x<0”);
  }
  return sqrt(x);
}

std::future<double> f=std::async(square_root,-1);
double y=f.get(); // 如果异步函数有抛异常，那么在调用get()函数时抛出被存储为future对象的异常

//promise也可以存入一个异常
extern std::promise<double> some_promise;
try
{
  some_promise.set_value(calculate_value());
}
catch(...)
{
  some_promise.set_exception(std::current_exception());
}