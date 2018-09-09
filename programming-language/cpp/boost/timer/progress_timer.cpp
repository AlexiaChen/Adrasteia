// boost::progress_timer使用比timer方便

#include <iostream>
#include <sstream>
#include <cstdint>
#include <boost/progress.hpp>

int main()
{
    {
        boost::progress_timer t;
        int sum = 0;
        for(int i = 0; i < 10000; ++i){ sum = sum + i;}
        std::cout << "sum: " << sum << std::endl;
        // progress_timer 离开作用域时析构，自动输出时间
    }

    {
        boost::progress_timer t;
        uint64_t sum = 0;
        for(int i = 0; i < 1000000000; ++i){ sum = sum + i; }
        std::cout << "sum: " << sum << std::endl;
        // progress_timer 离开作用域时析构，自动输出时间
    }
    
    std::stringstream ss;
    {
        boost::progress_timer t3(ss); // 要求progress_timer输出到ss字符串流中
        uint64_t sum = 0;
        for(int i = 0; i < 1000000000; ++i){ sum = sum + i; }
        std::cout << "sum: " << sum << std::endl;
        // progress_timer 离开作用域时析构，自动输出时间
    }

    std::cout << "t3 elapsed time is: " <<ss.str() << std::endl;
    
    return 0;
}