#include <iostream>
#include <vector>
#include <fstream>
#include <boost/progress.hpp>

int main()
{
    std::vector<std::string> v(10000000,"Hi, I'am MathxH, greetings from UC Berkeley. Where are you from?");
    std::ofstream fs("./test.txt");

    boost::progress_display pd(v.size()); // 构造pd的时候需要传入工作总量

    for(const auto& x : v)
    {
        fs << x << std::endl;
        ++pd;
    }

    // 自动显示写入文件进度

    return 0;
}