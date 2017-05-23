/*
Hamming Distance
URL: https://leetcode.com/problems/hamming-distance/#/description


Note:
0 ≤ x, y < 2^31 

Input: x = 1, y = 4

Output: 2

Explanation:
1   (0 0 0 1)
4   (0 1 0 0)
       ↑   ↑

The above arrows point to positions where the corresponding bits are different.
*/
#include <iostream>

using namespace std;

class Solution {
public:
    int hammingDistance(int x, int y) {
        return bitCount2(x ^ y);
    }
private:
    int bitCount1(int number)
    {
        int count = 0;
        for (int i = 0; i < 32; ++i)
        {
            auto temp = number >> i;
            if ((temp & 0x1) == 1) count++;
        }

        return count;
    }

    // Algorithm Expalnation: https://stackoverflow.com/questions/30455472/bitcount-method
    int bitCount2(int number)
    {
        // the algorithm from Hacker's Delight, Figure 5-2
        number -= (number >> 1) & 0x55555555;
        number = (number & 0x33333333) + ((number >> 2) & 0x33333333);
        number = ((number >> 4) + number) & 0x0F0F0F0F;
        number += number >> 8;
        number += number >> 16;
        return number & 0x0000003F;
    }
};

int main()
{
    Solution sln;
    std::cout << sln.hammingDistance(1, 4) << std::endl;
    
    int d;
    std::cin >> d;
    
    return 0;
}