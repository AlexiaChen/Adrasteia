/*
同步并发操作

使用消息传递的同步操作

CSP的概念十分简单：当没有共享数据，每个线程就可以进行独立思考，其行为纯粹基于其所接收到的信息。
每个线程就都有一个状态机：当线程收到一条信息，它将会以某种方式更新其状态，并且可能向其他线程发出一条或多条信息，
对于消息的处理依赖于线程的初始化状态。

真正通讯顺序处理是没有共享数据的，所有消息都是通过消息队列传递，但是因为C++线程共享一块地址空间，
所以达不到真正通讯顺序处理的要求。

这里就需要有一些约定了：作为一款应用或者是一个库的作者，我们有责任确保在我们的实现中，线程不存在共享数据。
当然，为了线程间的通信，消息队列是必须要共享的
*/

struct card_inserted
{
  std::string account;
};

class atm
{
  messaging::receiver incoming;
  messaging::sender bank;
  messaging::sender interface_hardware;
  void (atm::*state)(); //状态函数指针，每个状态关联一个状态处理函数

  std::string account;
  std::string pin;

  void waiting_for_card()  
  {
    interface_hardware.send(display_enter_card());  // 它发送一条信息到接口，让终端显示“等待卡片”的信息
    incoming.wait().  // 等待传入一条消息进行处理
      handle<card_inserted>(
      [&](card_inserted const& msg)  // 这里处理的消息类型只能是card_inserted类的，这里使用一个lambda函数对其进行处理
      {
       account=msg.account;
       pin=""; // 清空pin
       interface_hardware.send(display_enter_pin()); //再发送一条消息到硬件接口，让显示界面提示用户输入PIN
       state=&atm::getting_pin;
      }
    );
    // 当消息处理程序结束，状态函数就会返回，然后主循环会调用新的状态函数
  }
  void getting_pin()
  {
      incoming.wait()
    .handle<digit_pressed>(  // 等待用户输入完PIN的消息
      [&](digit_pressed const& msg)
      {
        unsigned const pin_length=4;
        pin+=msg.digit;
        if(pin.length()==pin_length)
        {
          bank.send(verify_pin(account,pin,incoming)); //向银行线程发送校验PIN是否正确的消息
          state=&atm::verifying_pin; // 把状态置为verifying_pin 
        }
      }
      )
    .handle<clear_last_pressed>(  // 删除最后一个输入的PIN字符，
      [&](clear_last_pressed const& msg)
      {
        if(!pin.empty())
        {
          pin.resize(pin.length()-1);
        }
      }
      )
    .handle<cancel_pressed>(  // 取消输入PIN
      [&](cancel_pressed const& msg)
      {
        state=&atm::done_processing;
      }
      );
  }
public:
  void run()  // 这是状态机入口
  {
    state=&atm::waiting_for_card;  // 初始化waiting_for_card的状态
    try
    {
      for(;;)
      {
        (this->*state)();  // 反复执行当前状态函数
      }
    }
    catch(messaging::close_queue const&)
    {
    }
  }
};

