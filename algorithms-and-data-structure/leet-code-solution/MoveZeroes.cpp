/*
Move Zeroes
URL: https://leetcode.com/problems/move-zeroes/#/description

Given an array nums, write a function to move all 0's to the end of it while maintaining the relative order of the non-zero elements.

For example, given nums = [0, 1, 0, 3, 12], after calling your function, nums should be [1, 3, 12, 0, 0].

Note:

You must do this in-place without making a copy of the array.
Minimize the total number of operations.

*/
#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;


class Solution {

public:
    void moveZeroes(vector<int>& nums) {
        int zero_count = 0;
        nums.erase(std::remove_if(nums.begin(), nums.end(), 
            [&](int ele) -> bool{ 
            if (ele == 0)
            {
                zero_count++;
                return true;
            }
            else
            {
                return false;
            }
        }
        ), nums.end());

        for (int i = 0; i < zero_count; ++i) nums.push_back(0);
    }
private:
   
};


int main()
{
   
    std::vector<int> nums = { 0, 1, 0, 3, 12 };
    Solution sln;
    sln.moveZeroes(nums);
    int f;
    std::cin >> f;
    
    return 0;
}
