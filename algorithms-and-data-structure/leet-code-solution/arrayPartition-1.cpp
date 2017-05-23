/*
Array Partition I
URL: https://leetcode.com/problems/array-partition-i/#/description


Input: [1,4,3,2]

Output: 4
Explanation: n is 2, and the maximum sum of pairs is 4.
*/
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Solution {
public:
    int arrayPairSum(vector<int>& nums) {
        std::sort(nums.begin(), nums.end());
        int sum = 0;
        for (auto iter = nums.begin(); iter != nums.end(); iter += 2)
        {
            sum += *iter;
        }
        return sum;
    }
};

int main()
{
    std::vector<int> nums = { 1, 4, 3, 2 };
    Solution sln;
    std::cout << sln.arrayPairSum(nums) << std::endl;
    
    int d;
    std::cin >> d;
    
    return 0;
}