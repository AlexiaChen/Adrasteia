/*
保护共享数据
发现接口内在的条件竞争

因为使用了互斥量或其他机制保护了共享数据，就不必再为条件竞争所担忧吗？
并不是，依旧需要确定数据受到了保护。
*/

#include <thread>
#include <mutex>
#include <exception>
#include <memory>
#include <stack> 

template<typename T,typename Container=std::deque<T> >
class bad_stack
{
public:
  explicit stack(const Container&);
  explicit stack(Container&& = Container());
  template <class Alloc> explicit stack(const Alloc&);
  template <class Alloc> stack(const Container&, const Alloc&);
  template <class Alloc> stack(Container&&, const Alloc&);
  template <class Alloc> stack(stack&&, const Alloc&);

  bool empty() const;
  size_t size() const;
  T& top();
  T const& top() const;
  void push(T const&);
  void push(T&&);
  void pop();
  void swap(stack&&);
private:
    std::mutex mutex_; //保护内部数据的互斥量
};

static void test1()
{
    bad_stack<int> s; //如果多线程并发访问这个对象s
    
    /*
    这样的调用顺序就不再安全了，因为在调用empty()和调用top()之间，
    可能有来自另一个线程的pop()调用并删除了最后一个元素。
    这是一个经典的条件竞争，使用互斥量对栈内部数据进行保护，
    但依旧不能阻止条件竞争的发生，这就是接口固有的问题。
    */
    if (! s.empty())
    {    
        int const value = s.top();    //未定义行为
        s.pop();    
        do_something(value);
    }
}


// 接口改进后的stack类
struct empty_stack: std::exception
{
  const char* what() const throw(){
    return "empty stack!";
  };
};

template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data_;
    mutable std::mutex m_;
public:
  threadsafe_stack():data_(std::stack<T>()){}
  threadsafe_stack(const threadsafe_stack& other)
  {
      std::lock_guard<std::mutex> lock(other.m_);
      data_ = other.data_; //在构造函数体中的执行拷贝
  }
  threadsafe_stack& operator=(const threadsafe_stack&) = delete; //赋值操作被删除

  void push(T new_value)
  {
      std::lock_guard<std::mutex> lock(m_);
      data_.push(new_value);
  }
  
  std::shared_ptr<T> pop()
  {
      std::lock_guard<std::mutex> lock(m_);
      if(data_.empty()) throw empty_stack(); // 在调用pop前，检查栈是否为空
      
      std::shared_ptr<T> const res(std::make_shared<T>(data_.top())); // 在修改堆栈前，分配出返回值
      data_.pop();
      return res;
  }
  void pop(T& value)
  {
      std::lock_guard<std::mutex> lock(m_);
      if(data_.empty()) throw empty_stack();
      value = data_.top();
      data_.pop();
  }
  bool empty() const
  {
      std::lock_guard<std::mutex> lock(m_);
      return data_.empty();
  }
};

static void test2()
{
     threadsafe_stack<int> s; //如果多线程并发访问这个对象s
    
    if (! s.empty())
    {    
        //即使还没有执行pop的时候，有另外的线程pop出了最后一个对象，那么以下pop也会抛出empty_stack异常
        s.pop(value);    
        do_something(value);
    }
}

int main()
{
    test1();
    test2();
    return 0;
}