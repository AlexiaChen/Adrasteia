/*
同步并发操作

使用同步操作简化代码

*/

//使用“期望”的函数化编程

/*
promise作为拼图的最后一块，它使得函数化编程模式并发化(FP-style concurrency)在C++中成为可能；
一个promise对象可以在线程间互相传递，并允许其中一个计算结果依赖于另外一个的结果，而非对共享数据的显式访问。
*/

//快速排序——顺序实现版
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
  if(input.empty())
  {
    return input;
  }
  std::list<T> result;
  result.splice(result.begin(),input,input.begin());  // 将input list的第一个元素移动到入result中
  T const& pivot=*result.begin();  // 将result中的第一个元素作为中间值

  auto divide_point=std::partition(input.begin(),input.end(),
             [&](T const& t){return t<pivot;});  // 把所有小于pivot的值放在pivot的前面，并返回一个不小于pivot值的迭代器

  std::list<T> lower_part;
  lower_part.splice(lower_part.end(),input,input.begin(),
             divide_point);  // 把所有小于pivot的值的序列移动到新的lower_part序列中，此刻input就包括了pivot和大于它的值序列
  auto new_lower(
             sequential_quick_sort(std::move(lower_part)));  // 返回已排好序的子序列lower_part
  auto new_higher(
             sequential_quick_sort(std::move(input)));  // 返回已排好序的子序列input（higher_part）

  result.splice(result.end(),new_higher);  // 将大于pivot的序列插入result的末尾
  result.splice(result.begin(),new_lower);  // 将小于pivot的序列插入result的首部，也就是pivot之前
  return result; // 整个序列已经有序
}


//快速排序 FP模式线程强化版
//因为使用函数化模式，所以使用“期望”很容易将其转化为并行的版本

/*
因为避开了共享易变数据，函数化编程可算作是并发编程的范型；
并且也是CSP(Communicating Sequential Processer,通讯顺序进程)的范型，这里线程理论上是完全分开的，
也就是没有共享数据，但是有通讯通道允许信息在不同线程间进行传递。
*/

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
  if(input.empty())
  {
    return input;
  }
  std::list<T> result;
  result.splice(result.begin(),input,input.begin());
  T const& pivot=*result.begin();

  auto divide_point=std::partition(input.begin(),input.end(),
                [&](T const& t){return t<pivot;});

  std::list<T> lower_part;
  lower_part.splice(lower_part.end(),input,input.begin(),
                divide_point);

  std::future<std::list<T> > new_lower(  // 启用一个异步任务排序小于pivot的序列就可以了，因为还有这个调用的主线程，共2个
                std::async(&parallel_quick_sort<T>,std::move(lower_part)));

  auto new_higher(
                parallel_quick_sort(std::move(input)));  // 对于大于pivot部分列表，如同之前一样，使用递归的方式进行排序

  result.splice(result.end(),new_higher);  
  result.splice(result.begin(),new_lower.get());  
  return result;
}