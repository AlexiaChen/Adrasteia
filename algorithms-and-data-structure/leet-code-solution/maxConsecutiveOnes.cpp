/*
Max Consecutive Ones 
URL: https://leetcode.com/problems/max-consecutive-ones/#/description

Given a binary array, find the maximum number of consecutive 1s in this array.

Example 1:

Input: [1,1,0,1,1,1]
Output: 3
Explanation: The first two digits or the last three digits are consecutive 1s.
    The maximum number of consecutive 1s is 3.

Note:

    The input array will only contain 0 and 1.
    The length of input array is a positive integer and will not exceed 10,000

*/

#include <iostream>
#include <algorithm>
#include <vector>

class Solution {
public:
    int findMaxConsecutiveOnes(std::vector<int>& nums) {
        int max = 0,  count = 0;
        for(auto const n : nums)
        {
            max = std::max(max, count = (n == 0) ? 0 :
