/*
基于锁的并发数据结构设计

基于锁设计更加复杂的数据结构
*/

//之前的栈和队列都很简单，接口相对固定，并且它们应用于比较特殊的情况。并不是所有数据结构都像它们一样简单；
//大多数数据结构支持更加多样化的操作。原则上，这将增大并行的可能性，但是也让对数据保护变得更加困难


//编写一个使用锁的线程安全查询表

/*
队列和栈一样，标准容器的接口不适合多线程进行并发访问，因为这些接口在设计的时候都存在固有的条件竞争，所以这些接口需要砍掉，以及重新修订。
*/

//为细粒度锁设计一个映射结构，为了允许细粒度锁能正常工作，需要对于数据结构的细节进行仔细的考虑，而非直接使用已存在的容器
//例如std::map等

//线程安全的查询表
class bucket_type
  {
  private:
    typedef std::pair<Key,Value> bucket_value;
    typedef std::list<bucket_value> bucket_data;
    typedef typename bucket_data::iterator bucket_iterator;

    bucket_data data;
    mutable boost::shared_mutex mutex;  // 用boost提供的机制实现读写锁

    bucket_iterator find_entry_for(Key const& key) const  // 2
    {
      return std::find_if(data.begin(),data.end(),
      [&](bucket_value const& item)
      {return item.first==key;});
    }
  public:
    Value value_for(Key const& key,Value const& default_value) const
    {
      boost::shared_lock<boost::shared_mutex> lock(mutex);  // 允许并发读
      bucket_iterator const found_entry=find_entry_for(key);
      return (found_entry==data.end())?
        default_value:found_entry->second;
    }

    void add_or_update_mapping(Key const& key,Value const& value)
    {
      std::unique_lock<boost::shared_mutex> lock(mutex);  // 不允许并发写，同一时刻只有一个写者
      bucket_iterator const found_entry=find_entry_for(key);
      if(found_entry==data.end())
      {
        data.push_back(bucket_value(key,value));
      }
      else
      {
        found_entry->second=value;
      }
    }

    void remove_mapping(Key const& key)
    {
      std::unique_lock<boost::shared_mutex> lock(mutex);  // 不允许并发删除，同一时刻只有一个删除者
      bucket_iterator const found_entry=find_entry_for(key);
      if(found_entry!=data.end())
      {
        data.erase(found_entry);
      }
    }
  };

template<typename Key,typename Value,typename Hash=std::hash<Key> >
class threadsafe_lookup_table
{
private:
  std::vector<std::unique_ptr<bucket_type> > buckets;  //一个查找表中有N个桶，一个桶中有N个key value对，打算采用分段锁
  Hash hasher;

  bucket_type& get_bucket(Key const& key) const  // 用hash给key映射到不同的桶中，实现分段加锁，减小锁的粒度
  {
    std::size_t const bucket_index = hasher(key) % buckets.size(); //size()为质数，效率最高
    return *buckets[bucket_index];
  }

public:
  typedef Key key_type;
  typedef Value mapped_type;

  typedef Hash hash_type;
  threadsafe_lookup_table(
    unsigned num_buckets=19,Hash const& hasher_=Hash()): //默认为19个，其是一个任意的质数;哈希表在有质数个桶时，工作效率最高
    buckets(num_buckets),hasher(hasher_)
  {
    for(unsigned i=0;i<num_buckets;++i)
    {
      buckets[i].reset(new bucket_type);
    }
  }

  threadsafe_lookup_table(threadsafe_lookup_table const& other)=delete;
  threadsafe_lookup_table& operator=(
    threadsafe_lookup_table const& other)=delete;

  Value value_for(Key const& key,
                  Value const& default_value=Value()) const
  {
    return get_bucket(key).value_for(key,default_value);  // 先找到key所在的对应的桶，再在桶中线性搜索KV对
  }

  void add_or_update_mapping(Key const& key,Value const& value)
  {
    get_bucket(key).add_or_update_mapping(key,value);  // 同上，修改新增
  }

  void remove_mapping(Key const& key)
  {
    get_bucket(key).remove_mapping(key);  //  同上，删除
  }

  std::map<Key,Value> get_map() const
  {
    std::vector<std::unique_lock<boost::shared_mutex> > locks;
    for(unsigned i=0;i<buckets.size();++i)
    {
        locks.push_back(
        std::unique_lock<boost::shared_mutex>(buckets[i].mutex));
    }
    std::map<Key,Value> res;
    for(unsigned i=0;i<buckets.size();++i)
    {
        for(bucket_iterator it=buckets[i].data.begin();
            it!=buckets[i].data.end();
            ++it)
        {
          res.insert(*it);
        }
    }
    return res;
  }
};




