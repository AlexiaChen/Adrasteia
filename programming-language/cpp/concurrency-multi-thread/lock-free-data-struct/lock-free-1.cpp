/*
无锁并发数据结构设计

作为无锁结构，就意味着线程可以并发的访问这个数据结构。线程不能做相同的操作；一个无锁队列可能允许一个线程进行压入数据，
另一个线程弹出数据，当有两个线程同时尝试添加元素时，这个数据结构将被破坏。

无等待数据结构就是：首先，是无锁数据结构；并且，每个线程都能在有限的步数内完成操作，暂且不管其他线程是如何工作的。


使用无锁结构的主要原因：将并发最大化。使用基于锁的容器，会让线程阻塞或等待；互斥锁削弱了结构的并发性。在无锁数据结构中，某些线程可以逐步执行。
在无等待数据结构中，无论其他线程当时在做什么，每一个线程都可以转发进度。这种理想的方式实现起来很难。

使用无锁数据结构的第二个原因就是鲁棒性。当一个线程在获取一个锁时被杀死，那么数据结构将被永久性的破坏。不过，当线程在无锁数据结构上执行操作，
在执行到一半死亡时，数据结构上的数据没有丢失(除了线程本身的数据)，其他线程依旧可以正常执行。

“无锁-无等待”代码的缺点：虽然提高了并发访问的能力，减少了单个线程的等待时间，但是其可能会将整体性能拉低。首先，
原子操作的无锁代码要慢于无原子操作的代码，原子操作就相当于无锁数据结构中的锁。不仅如此，硬件必须通过同一个原子变量对线程间的数据进行同步。
*/


/*
无锁结构依赖与原子操作和内存序及相关保证，以确保多线程以正确的顺序访问数据结构。最初，C++ 11所有原子操作默认使用的是memory_order_seq_cst内存序；
因为简单，所以使用(所有memory_order_seq_cst都遵循一种顺序)。不过，在后面的例子中，我们将会降低内存序的要求，使用memory_order_acquire, 
memory_order_release, 甚至memory_order_relaxed。
*/


//一个无锁的线程安全栈
//结构是无锁的（Lock Free），但并不是无等待的（Wait Free），因为在push()和pop()函数中都有while循环，
//当compare_exchange_weak()总是失败的时候，循环将会无限循环下去。
//而且在pop时还有微妙的内存泄漏
template<typename T>
class lock_free_stack
{
private:
  struct node
  {
    T data;
    node* next;

    node(T const& data_):  
     data(data_)
    {}
  };

  std::atomic<node*> head; //把共享变量设置为原子类型的指针
public:
  void push(T const& data)
  {
    node* const new_node = new node(data); //唯一能抛出异常的地方就构造新node的时候，不过其会自行处理，且链表中的内容没有被修改，所以这里是安全的。
    new_node->next=head.load();  // 设置新节点的next指针指向当前head
    
    /*
      并设置head指针指向新节点，先对比head与new_node->next是否相等，如果不相等，返回false，再次继续重新读取head值循环对比，
      直到相等成功返回true，第二个参数new_node被存储到head中，也就是head指向new_node，跳出循环
    */
    while(!head.compare_exchange_weak(new_node->next,new_node));  
  }

  std::shared_ptr<T> pop()
  {
      node* old_head=head.load();
      /*
        当“比较/交换”操作失败时，不是一个新节点已被推入，就是其他线程已经弹出了想要弹出的节点。
      */
      while(old_head && // 在解引用前检查old_head是否为空指针
      !head.compare_exchange_weak(old_head,old_head->next));
      return old_head ? old_head->data : std::shared_ptr<T>();  
  }


};

//停止内存泄露：使用无锁数据结构管理内存
template<typename T>
class lock_free_stack
{
private:
  std::atomic<unsigned> threads_in_pop;  // 1 原子变量
  std::atomic<node*> to_be_deleted;

  void chain_pending_nodes(node* nodes)
  {
    node* last=nodes;
    while(node* const next=last->next)  // 9 让next指针指向链表的末尾
    {
      last=next;
    }
    chain_pending_nodes(nodes,last);
  }

  void chain_pending_nodes(node* first,node* last)
  {
    last->next=to_be_deleted;  // 10
    while(!to_be_deleted.compare_exchange_weak(  // 11 用循环来保证last->next的正确性
      last->next,first));
  }
  
  void chain_pending_node(node* n)
  {
      chain_pending_nodes(n,n);  // 12
  }
  
  void try_reclaim(node* old_head)
  {
    if(threads_in_pop==1)  // 1
    {
      node* nodes_to_delete=to_be_deleted.exchange(nullptr);  // 2 声明“可删除”列表
      if(!--threads_in_pop)  // 3 是否只有一个线程调用pop()？
      {
        delete_nodes(nodes_to_delete);  // 4
      }
      else if(nodes_to_delete)  // 5
      {
         chain_pending_nodes(nodes_to_delete);  // 6
      }
      delete old_head;  // 7
    }
    else
    {
      chain_pending_node(old_head);  // 8
      --threads_in_pop;
    }
  }
public:
  std::shared_ptr<T> pop()
  {
    ++threads_in_pop;  // 2 在做事之前，计数值加1
    node* old_head=head.load();
    while(old_head &&
      !head.compare_exchange_weak(old_head,old_head->next));
    std::shared_ptr<T> res;
    if(old_head)
    { 
      res.swap(old_head->data);  // 3 回收删除的节点
    }
    try_reclaim(old_head);  // 4 从节点中直接提取数据，而非拷贝指针
    return res;
  }
};

//检测使用风险指针(不可回收)的节点
unsigned const max_hazard_pointers=100;
struct hazard_pointer
{
  std::atomic<std::thread::id> id;
  std::atomic<void*> pointer;
};
hazard_pointer hazard_pointers[max_hazard_pointers];

class hp_owner
{
  hazard_pointer* hp;

public:
  hp_owner(hp_owner const&)=delete;
  hp_owner operator=(hp_owner const&)=delete;
  hp_owner():
    hp(nullptr)
  {
    for(unsigned i=0;i<max_hazard_pointers;++i)
    {
      std::thread::id old_id;
      if(hazard_pointers[i].id.compare_exchange_strong(  // 6 尝试声明风险指针的所有权
         old_id,std::this_thread::get_id()))
      {
        hp=&hazard_pointers[i];
        break;  // 7
      }
    }
    if(!hp)  // 1
    {
      throw std::runtime_error("No hazard pointers available");
    }
  }

  std::atomic<void*>& get_pointer()
  {
    return hp->pointer;
  }

  ~hp_owner()  // 2
  {
    hp->pointer.store(nullptr);  // 8
    hp->id.store(std::thread::id());  // 9
  }
};

std::atomic<void*>& get_hazard_pointer_for_current_thread()  // 3
{
  thread_local static hp_owner hazard;  // 4 每个线程都有自己的风险指针
  return hazard.get_pointer();  // 5
}

bool outstanding_hazard_pointers_for(void* p)
{
  for(unsigned i=0;i<max_hazard_pointers;++i)
  {
    if(hazard_pointers[i].pointer.load()==p)
    {
      return true;
    }
  }
  return false;
}


template<typename T>
class lock_free_stack
{
private:
  std::atomic<unsigned> threads_in_pop;  // 1 原子变量
  std::atomic<node*> to_be_deleted;

 template<typename T>
void do_delete(void* p)
{
  delete static_cast<T*>(p);
}

struct data_to_reclaim
{
  void* data;
  std::function<void(void*)> deleter;
  data_to_reclaim* next;

  template<typename T>
  data_to_reclaim(T* p):  // 1
    data(p),
    deleter(&do_delete<T>),
    next(0)
  {}

  ~data_to_reclaim()
  {
    deleter(data);  // 2
  }
};

std::atomic<data_to_reclaim*> nodes_to_reclaim;

void add_to_reclaim_list(data_to_reclaim* node)  // 3
{
  node->next=nodes_to_reclaim.load();
  while(!nodes_to_reclaim.compare_exchange_weak(node->next,node));
}

template<typename T>
void reclaim_later(T* data)  // 4
{
  add_to_reclaim_list(new data_to_reclaim(data));  // 5
}

void delete_nodes_with_no_hazards()
{
  data_to_reclaim* current=nodes_to_reclaim.exchange(nullptr);  // 6
  while(current)
  {
    data_to_reclaim* const next=current->next;
    if(!outstanding_hazard_pointers_for(current->data))  // 7
    {
      delete current;  // 8
    }
    else
    {
      add_to_reclaim_list(current);  // 9
    }
    current=next;
  }
}
public:
  std::shared_ptr<T> pop()
  {
    std::atomic<void*>& hp=get_hazard_pointer_for_current_thread();
  node* old_head=head.load();
  do
  {
    node* temp;
    do  // 1 直到将风险指针设为head指针
    {
      temp=old_head;
      hp.store(old_head);
      old_head=head.load();
    } while(old_head!=temp);
  }
  while(old_head &&
    !head.compare_exchange_strong(old_head,old_head->next));
  hp.store(nullptr);  // 2 当声明完成，清除风险指针
  std::shared_ptr<T> res;
  if(old_head)
  {
    res.swap(old_head->data);
    if(outstanding_hazard_pointers_for(old_head))  // 3 在删除之前对风险指针引用的节点进行检查
    {
      reclaim_later(old_head);  // 4
    }
    else
    {
      delete old_head;  // 5
    }
    delete_nodes_with_no_hazards();  // 6
  }
  return res;
  }
};
