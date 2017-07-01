/*
Single Number
URL: https://leetcode.com/problems/single-number/#/description

Given an array of integers, every element appears twice except for one. Find that single one.

Note:
Your algorithm should have a linear runtime complexity. Could you implement it without using extra memory?
*/
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;


class Solution {
public:
    int singleNumber(vector<int>& nums) {

        std::unordered_map<int, int> map;
        for (const int num : nums)
        {
            map[num]++;
        }

        for (const int num : nums)
        {
            if (map[num] == 1) return num;
        }

        return 0;
    }
};


int main()
{
   
    Solution sln;
    std::vector<int> nums = { 2, 3, 3, 2, 6, 7, 8, 8, 7, 6, 5 };
    std::cout << sln.singleNumber(nums);

    int f;
    std::cin >> f;
    
    return 0;
}
