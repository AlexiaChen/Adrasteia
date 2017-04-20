/*
C++ 11的线程管理
运行时决定线程数量
*/

#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <numeric>


/*
一个并行版的std::accumulate。代码中将整体工作拆分成小任务交给每个线程去做，
其中设置最小任务数，是为了避免产生太多的线程。
*/
template<typename Iterator,typename T>
struct accumulate_block
{
  void operator()(Iterator first,Iterator last,T& result)
  {
    result = std::accumulate(first,last,result);
  }
};

template<typename Iterator,typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
  unsigned long const length = std::distance(first,last);

  if(!length) // 如果输入的范围为空，就会得到init的值
    return init;

  unsigned long const min_per_thread = 25; // 单个线程中最小任务数
  
  // 如果范围内多于一个元素时，都需要用范围内元素的总数量除以线程(块)中最小任务数，从而确定启动线程的最大数量
  unsigned long const max_threads=
      (length + min_per_thread - 1)/min_per_thread; 

  //多核系统中，返回值可以是CPU核芯的数量。返回值也仅仅是一个提示，当系统信息无法获取时，函数也会返回0。
  unsigned long const hardware_threads=
      std::thread::hardware_concurrency();// 返回能同时并发在一个程序中的线程数量

  unsigned long const num_threads=  
      std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

  unsigned long const block_size = length/num_threads; // 每个线程中处理的元素数量,是范围中元素的总量除以线程的个数得出的

  std::vector<T> results(num_threads);
  std::vector<std::thread> threads(num_threads-1);  // 启动的线程数必须比num_threads少1个，因为在启动之前已经有了一个主线程

  Iterator block_start = first;
  for(unsigned long i = 0; i < (num_threads - 1); ++i)
  {
    Iterator block_end = block_start;
    std::advance(block_end,block_size);  // block_end迭代器指向当前块的末尾
    threads[i]=std::thread(     // 把当前的block放入新线程计算
        accumulate_block<Iterator,T>(),
        block_start,block_end,std::ref(results[i]));
    block_start = block_end;  // 当迭代器指向当前块的末尾时，启动下一个块
  }
  accumulate_block<Iterator,T>()(
      block_start,last,results[num_threads-1]); //用主线程计算最后一个block
  std::for_each(threads.begin(),threads.end(),
       std::mem_fn(&std::thread::join));  

  return std::accumulate(results.begin(),results.end(),init); // 把所有分块的结果累计
}

int main()
{
    std::vector<int> v;
    for(unsigned int i = 1; i <= 1000; ++i) v.push_back(i);
    int r = parallel_accumulate<std::vector<int>::Iterator,int>(v.begin(),v.end(),0);    
    return 0;
}