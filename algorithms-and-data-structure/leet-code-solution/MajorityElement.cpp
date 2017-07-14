/*
Majority Element
URL: https://leetcode.com/problems/majority-element/#/description

Given an array of size n, find the majority element. The majority element is the element that appears more than ⌊ n/2 ⌋ times.

You may assume that the array is non-empty and the majority element always exist in the array.

*/
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <cmath>
#include <string>

using namespace std;


class Solution {

public:
    int majorityElement(vector<int>& nums) {
        int halfTimes = nums.size() / 2;
        std::unordered_map<int, int> counts;
        for (int num : nums)
        {
            if (++counts[num] > halfTimes) return num;
        }
        return 0;
    }
private:
     
};
