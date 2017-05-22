/*
Two Sum
URL: https://leetcode.com/problems/two-sum/#/description

Given nums = [2, 7, 11, 15], target = 9,

Because nums[0] + nums[1] = 2 + 7 = 9,
return [0, 1].
*/
#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;

class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        std::unordered_map<int, int> hash_table;
        std::vector<int> result;
        for (int i = 0; i < nums.size(); ++i)
        {
            int num_to_find = target - nums[i];
            if (hash_table.find(num_to_find) != hash_table.end())
            {
                result.push_back(hash_table[num_to_find]);
                result.push_back(i);
            }

            hash_table[nums[i]] = i;
        }

        return result;

    }
};

int main()
{
    
    vector<int> nums = { 2, 7, 11, 15 };
    int target = 26;
    Solution sln;
    auto result = sln.twoSum(nums, target);
    for (const auto i : result)
    {
        std::cout << i << " ";
    }

    int d;
    std::cin >> d;
    
    return 0;
}