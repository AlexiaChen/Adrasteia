/*
保护共享数据

保护很少更新的数据结构
支持递归的锁
*/

//由于DNS缓存更新少，读取多，所以用std::mutex独占锁住整个
//DNS缓存，虽然线程安全了，但是并发量上不去
//所以采用读写锁，把写行为与读行为分离了，适用于读多写少的情况

//标准没有实现读写锁，但是boost实现了

#include <map> //用map来充当DNS cache xxx.com -> 10.2.13.56
#include <string>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>

class dns_entry;

class dns_cache
{
  std::map<std::string,dns_entry> entries;
  mutable boost::shared_mutex entry_mutex;
public:
  dns_entry find_entry(std::string const& domain) const
  {
    // 使用boost::shared_lock<>来保护共享和只读权限；这就使得多线程可以同时调用find_entry()，且不会出错
    // 这是读锁，共享锁
    boost::shared_lock<boost::shared_mutex> lk(entry_mutex);  
    std::map<std::string,dns_entry>::const_iterator const it=
       entries.find(domain);
    return (it==entries.end())?dns_entry():it->second;
  }
  void update_or_add_entry(std::string const& domain,
                           dns_entry const& dns_details)
  {
    /*
    update_or_add_entry()使用std::lock_guard<>实例，当DNS缓存需要更新时，为其提供独占访问权限；
    update_or_add_entry()函数调用时，独占锁会阻止其他线程对数据结构进行修改，并且阻止线程调用find_entry()。
    */
    std::lock_guard<boost::shared_mutex> lk(entry_mutex);  // 2
    entries[domain]=dns_details;
  }
};


//std::mutex vs std::recursive_mutex使用场景对比
// 多线程并发访问foo，并且foo递归调用了自身
std::recursive_mutex m_;
void foo() {
    std::lock_guard<std::recursive_mutex> lock(m_);
    do_something();
    foo();
    do_something();
}

//如果不使用std::recursive_mutex,而使用std::mutex,这样就闲得很笨重
std::mutex m_;
void foo_entry()
{
    std::lock_guard<std::mutex> lock(m_);
    foo();
}
void foo() {
    do_something();
    foo();
    do_something();
}

//在采用递归的算法的数据结构中，需要并发访问的数据结构
// 以下代码是以上对比的更加细节化

// 采用了std::recursive_mutex 的二叉搜索树
haystack_pointer search(haystack, mutex, needle)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (haystack == NULL)
    {
        return NULL;
    }

    if (haystack.payload == needle) return haystack;

    if (haystack.payload > needle)
    {
        found = search (haystack.left, mutex, needle);
    }
    else
    {
        found = search (haystack.right, mutex, needle);
    }

    return found
}

std::recursive_mutex m_;
search(haystack,m_,need_data);

//// 采用了std::mutex 的二叉搜索树
haystack_pointer search_locked (haystack, needle)
{
    if (haystack == NULL) return NULL;
    if (haystack.payload == needle) return haystack;

    if (haystack.payload > needle)
    {
        found = search_locked (haystack.left, needle);
    }
    else
    {
        found = search_locked (haystack.right, needle);
    }

    return found

}

haystack_pointer search (haystack, mutex, needle)
{
    std::lock_guard<std::mutex> lock(mutex);
    return search_locked (haystack, needle);
}
    