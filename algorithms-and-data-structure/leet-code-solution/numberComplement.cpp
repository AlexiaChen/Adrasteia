/*
Number Complement  (补码)
URL: https://leetcode.com/problems/number-complement/#/description


Note:

The given integer is guaranteed to fit within the range of a 32-bit signed integer.
You could assume no leading zero bit in the integer’s binary representation.

Example 1:

Input: 5
Output: 2
Explanation: The binary representation of 5 is 101 (no leading zero bits), and its complement is 010. So you need to output 2.

Example 2:

Input: 1
Output: 0
Explanation: The binary representation of 1 is 1 (no leading zero bits), and its complement is 0. So you need to output 0.

*/
#include <iostream>
#include <cstdint>

using namespace std;

class Solution {
public:
    int findComplement(int num) {
        uint32_t mask = ~0; // 0xFFFFFFFF
        while (num & mask) mask <<= 1; // prepare for clear leading zero
        return ~num & ~mask;
    }
};

int main()
{
    int num = 1;
    Solution sln;
    std::cout << sln.findComplement(num) << std::endl;
    
    int d;
    std::cin >> d;
    
    return 0;
}