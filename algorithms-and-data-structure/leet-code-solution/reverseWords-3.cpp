/*
Reverse Words in a String III
URL: https://leetcode.com/problems/reverse-words-in-a-string-iii/#/description

Example 1:

Input: "Let's take LeetCode contest"
Output: "s'teL ekat edoCteeL tsetnoc"

Note: In the string, each word is separated by single space and there will not be any extra space in the string.
*/
#include <iostream>
#include <string>
#include <stack>

using namespace std;

class Solution {
public:
    string reverseWords(string s) {
        std::string r;
        std::stack<char> stk;
        for (int i = 0; i < s.length(); ++i)
        {
            if (s[i] != ' ')
            {
                stk.push(s[i]);
            }
            else
            {
                while (!stk.empty())
                {
                    r.push_back(stk.top());
                    stk.pop();
                }
                r.push_back(' ');
            }
        }

        while (!stk.empty())
        {
            r.push_back(stk.top());
            stk.pop();
        }

        return r;
    }
};

int main()
{
    std::string str = "Let's take LeetCode contest";
    Solution sln;
    std::cout << sln.reverseWords(str) << std::endl;
    
    int d;
    std::cin >> d;
    
    return 0;
}