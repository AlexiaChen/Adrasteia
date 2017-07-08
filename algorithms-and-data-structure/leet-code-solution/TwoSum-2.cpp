/*
Two Sum II - Input array is sorted
URL: https://leetcode.com/problems/two-sum-ii-input-array-is-sorted/#/description

Given an array of integers that is already sorted in ascending order, find two numbers such that they add up to a specific target number.

The function twoSum should return indices of the two numbers such that they add up to the target, where index1 must be less than index2. Please note that your returned answers (both index1 and index2) are not zero-based.

You may assume that each input would have exactly one solution and you may not use the same element twice.

Input: numbers={2, 7, 11, 15}, target=9
Output: index1=1, index2=2
*/
#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cmath>

using namespace std;


class Solution {

public:
    vector<int> twoSum(vector<int>& numbers, int target) {
        std::unordered_map<int, std::pair<int,bool>> num_index_lookup;
        for (int i = 0; i < numbers.size(); ++i)
        {
            num_index_lookup[numbers[i]] = std::make_pair(i,true);
        }

        for (int i = 0; i < numbers.size(); ++i)
        {
            int diff = target - numbers[i];
            int diff_index_exists = num_index_lookup[diff].second;
            int diff_index = num_index_lookup[diff].first;
            if (diff_index_exists)
            {
                std::vector<int> ret{i + 1,diff_index + 1};
                return ret;
                
            }
        }

        return std::vector<int>(2);
    }
private:
   
};


int main()
{
   
   
    Solution sln;
    std::vector<int> vec{ 2, 7, 11, 15 };
    std::vector<int> vec1{ 2, 3, 4 };
    auto r1 = sln.twoSum(vec, 9);
    auto r2 = sln.twoSum(vec1, 6);
    int f;
    std::cin >> f;
    
    return 0;
}
