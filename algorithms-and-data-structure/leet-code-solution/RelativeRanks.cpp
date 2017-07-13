/*
Relative Ranks
URL: https://leetcode.com/problems/relative-ranks/#/description

Given scores of N athletes, find their relative ranks and the people with the top three highest scores, who will be awarded medals: "Gold Medal", "Silver Medal" and "Bronze Medal".

Example 1:

Input: [5, 4, 3, 2, 1]
Output: ["Gold Medal", "Silver Medal", "Bronze Medal", "4", "5"]
Explanation: The first three athletes got the top three highest scores, so they got "Gold Medal", "Silver Medal" and "Bronze Medal".
For the left two athletes, you just need to output their relative ranks according to their scores.

Note:

N is a positive integer and won't exceed 10,000.
All the scores of athletes are guaranteed to be unique.


*/
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <string>

using namespace std;


class Solution {

public:
    vector<string> findRelativeRanks(vector<int>& nums) {
        std::vector<int> copy_nums;
        std::copy(nums.begin(), nums.end(), std::back_inserter(copy_nums));
        std::sort(copy_nums.begin(), copy_nums.end(), std::greater<int>());
        
        std::unordered_map<int, std::string> rankMap;
        for (int i = 0; i < copy_nums.size(); ++i)
        {
            if (i == 0)
            {
                rankMap[copy_nums[i]] = "Gold Medal"; continue;
            }
            if (i == 1)
            {
                rankMap[copy_nums[i]] = "Silver Medal"; continue;
            }
            if (i == 2)
            {
                rankMap[copy_nums[i]] = "Bronze Medal"; continue;
            }

            rankMap[copy_nums[i]] = std::to_string(i + 1);

        }

        std::vector<std::string> ret;
        for (auto v : nums)
        {
            ret.push_back(rankMap[v]);
        }
        return ret;

    }
private:
     
};
