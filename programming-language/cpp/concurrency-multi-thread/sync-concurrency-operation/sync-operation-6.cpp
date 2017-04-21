/*
同步并发操作

直到现在，所有例子都在用std::future。不过，std::future也有局限性，在很多线程在等待的时候，只有一个线程能获取等待结果。
当多个线程需要等待相同的事件的结果，你就需要使用std::shared_future来替代std::future了。
*/

//多个线程的等待

std::promise<int> p;
std::future<int> f(p.get_future());
assert(f.valid());  // 1 "期望" f 是合法的
std::shared_future<int> sf(std::move(f));
assert(!f.valid());  // 2 "期望" f 现在是不合法的
assert(sf.valid());  // 3 sf 现在是合法的

//可以把shared_future 传递到多个线程中变为局部变量，shared_future允许拷贝

