/*
Fizz Buzz
URL: https://leetcode.com/problems/fizz-buzz/#/description

for multiples of three it should output “Fizz” instead of the number and for the multiples of five output “Buzz”. For numbers which are multiples of both three and five output “FizzBuzz”.

Example:

n = 15,

Return:
[
"1",
"2",
"Fizz",
"4",
"Buzz",
"Fizz",
"7",
"8",
"Fizz",
"Buzz",
"11",
"Fizz",
"13",
"14",
"FizzBuzz"
]


*/
#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Solution {
public:
    vector<string> fizzBuzz(int n) {
        std::vector<std::string> r;
        for (int i = 1; i <= n; ++i)
        {
            if (i % 3 == 0 && i % 5 == 0)
            {
                r.push_back("FizzBuzz");
            }
            else if (i % 3 == 0 && i % 5 != 0)
            {
                r.push_back("Fizz");
            }
            else if (i % 3 != 0 && i % 5 == 0)
            {
                r.push_back("Buzz");
            }
            else
            {
                r.push_back(std::to_string(i));
            }
        }
        return r;
    }
};

int main()
{
    int n;
    std::cin >> n;
    Solution sln;
    std::vector<std::string> r;
    r = sln.fizzBuzz(n);
    for each (std::string str in r)
    {
        std::cout << str << " ";
    }
 
    int d;
    std::cin >> d;
    
    return 0;
}