/*
Reverse String
URL: https://leetcode.com/problems/reverse-string/#/description

Example:
Given s = "hello", return "olleh".
*/
#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Solution {
public:
    string reverseString(string s) {
        std::string r;
        for (auto iter = s.rbegin(); iter != s.rend(); ++iter) r.push_back(*iter);
        return r;
    }
};
int main()
{
   
    Solution sln;
    std::string s = "Hello";
    std::cout << sln.reverseString(s) << std::endl;
    int d;
    std::cin >> d;
    
    return 0;
}