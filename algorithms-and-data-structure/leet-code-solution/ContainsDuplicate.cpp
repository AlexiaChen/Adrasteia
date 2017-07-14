/*
Contains Duplicate
URL: https://leetcode.com/problems/contains-duplicate/#/description

Given an array of integers, find if the array contains any duplicates. Your function should return true if any value appears at least twice 
in the array, and it should return false if every element is distinct.
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
    bool containsDuplicate(vector<int>& nums) {
        std::unordered_map<int, int> counts;
        for (int num : nums) counts[num]++;
        for (auto cnt : counts)
        {
            if (cnt.second > 1) return true;
        }
        return false;
    }
    
private:
     
};
