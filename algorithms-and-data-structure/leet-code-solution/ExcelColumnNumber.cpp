/*
Excel Sheet Column Number
URL: https://leetcode.com/problems/excel-sheet-column-number/#/description

Given a column title as appear in an Excel sheet, return its corresponding column number.

For example:

A -> 1
B -> 2
C -> 3
...
Z -> 26
AA -> 27
AB -> 28

*/
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>

using namespace std;


class Solution {

public:
    int titleToNumber(string s) {
        std::unordered_map<char, int> alphaMap(26);
        size_t len = s.length();
        int k = 1;
        for (char c = 'A'; c <= 'Z'; c++,k++)
        {
            alphaMap[c] = k;
        }

        int sum = 0;
        for (int i = 0; i < len; ++i)
        {
            sum += alphaMap[s[i]] * (int)std::pow<int,int>(26, len - (i+1));
        }

        return sum;
    }
private:
     
};


int main()
{
   
   
    Solution sln;

    std::cout << sln.titleToNumber("AA") << std::endl;
    std::cout << sln.titleToNumber("A") << std::endl;
    std::cout << sln.titleToNumber("AB") << std::endl;
    std::cout << sln.titleToNumber("Z") << std::endl;

   
    int f;
    std::cin >> f;
    
    return 0;
}
