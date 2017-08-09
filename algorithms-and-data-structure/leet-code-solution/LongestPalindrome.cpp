/*
Longest Palindrome
URL: https://leetcode.com/problems/longest-palindrome/description/

Given a string which consists of lowercase or uppercase letters, find the length of the longest palindromes that can be built with those letters.

This is case sensitive, for example "Aa" is not considered a palindrome here.

Note:
Assume the length of given string will not exceed 1,010.

Example:

Input:
"abccccdd"

Output:
7

Explanation:
One longest palindrome that can be built is "dccaccd", whose length is 7.

*/              
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <cmath>
#include <string>
#include <unordered_set>

using namespace std;


class Solution {

public:
    int longestPalindrome(string s) {
        if (s.empty()) return 0;
        int count = 0;
        std::unordered_set<char> ch_set;
        for (char c : s)
        {
            auto isFind = ch_set.find(c) != ch_set.end();
            if (isFind)
            {
                ch_set.erase(ch_set.find(c));
                count++; //count char pair
            }
            else
            {
                ch_set.insert(c);  
            }
        }
        return ch_set.empty() ? (count * 2) : (count * 2 + 1);
    }
   
};


int main()
{
   
   
    Solution sln;

    
    std::cout << sln.longestPalindrome("abccccdd");
    std::cout << sln.longestPalindrome("ccc");
    std::cout << sln.longestPalindrome("ddbddff");
   
    int f;
    std::cin >> f;
    
    return 0;
}
