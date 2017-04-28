/*
基于锁的并发数据结构设计

编写一个使用锁的线程安全链表
*/

//线程安全链表——支持迭代器
template<typename T>
class threadsafe_list
{
  struct node  // 单链表
  {
    std::mutex m;
    std::shared_ptr<T> data;
    std::unique_ptr<node> next;
    node():  // next指针为NULL
      next()
    {}

    node(T const& value):  
      data(std::make_shared<T>(value))
    {}
  };

  node head;

public:
  threadsafe_list()
  {}

  ~threadsafe_list()
  {
    remove_if([](node const&){return true;});
  }

  threadsafe_list(threadsafe_list const& other)=delete;
  threadsafe_list& operator=(threadsafe_list const& other)=delete;

  void push_front(T const& value)
  {
    std::unique_ptr<node> new_node(new node(value)); 
    std::lock_guard<std::mutex> lk(head.m); //锁可以不用加在分配new_node的时候，分配内存很耗时，只用保护好head指针
    new_node->next=std::move(head.next);  
    head.next=std::move(new_node);  
  }

  template<typename Function>
  void for_each(Function f)  
  {
    node* current=&head;
    std::unique_lock<std::mutex> lk(head.m);  // 先锁住head节点
    while(node* const next=current->next.get())  // 遍历
    {
      std::unique_lock<std::mutex> next_lk(next->m);  // 再锁住next节点
      lk.unlock();  // 解锁pre节点
      f(*next->data);  // 用callback处理当前节点的数据
      current=next;
      lk=std::move(next_lk);  // 再把当前节点的锁的所有权转移到lk上，如此迭代反复
    }
  }

  template<typename Predicate>
  std::shared_ptr<T> find_first_if(Predicate p)  
  {
    node* current=&head;
    std::unique_lock<std::mutex> lk(head.m);
    while(node* const next=current->next.get())
    {
      std::unique_lock<std::mutex> next_lk(next->m);
      lk.unlock();
      if(p(*next->data))  
      {
         return next->data;  
      }
      current=next;
      lk=std::move(next_lk);
    }
    return std::shared_ptr<T>();
  }

  template<typename Predicate>
  void remove_if(Predicate p)  // 这个函数会改变链表，所以就不能使用for_each()来实现这个功能
  {
    node * current = &head;
    std::lock_guard<std::mutex> lk(head.m);
    while(const node* next = current->next.get())
    {
        std::unique_lock<std::mutex> next_lk(next->m);
        if(p(*next->data))
        {
            std::unique_ptr<node> old_next = std::move(current->next); // 把current->next的所有权移走，才能重新对其赋值
            current->next = std::move(next->next); 
            next_lk.unlock();
        }
        else
        {
            lk.unlock();
            current = next;
            lk = std::move(next_lk);
        }
    }
};