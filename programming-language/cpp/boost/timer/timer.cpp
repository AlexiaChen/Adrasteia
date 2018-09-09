// 不适合高精度时间测量任务

#include <iostream>
#include <boost/timer.hpp>

int main()
{
    boost::timer t;

    std::cout << "Max timespan:" << t.elapsed_max() / 3600 << "hours" << std::endl;
    std::cout << "Min timespan:" << t.elapsed_min()  << "seconds" << std::endl;
    
    std::cout << "now time elapsed:" << t.elapsed() << "seconds" << std::endl;

    t.restart();

    for(int i = 0; i < 10000; ++i){}

    std::cout << "now time elapsed:" << t.elapsed() << "seconds" << std::endl;

    return 0;
}