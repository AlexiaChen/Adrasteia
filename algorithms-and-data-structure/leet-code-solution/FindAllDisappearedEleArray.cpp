/*
Find All Numbers Disappeared in an Array
URL: https://leetcode.com/problems/find-all-numbers-disappeared-in-an-array/#/description

Given an array of integers where 1 ≤ a[i] ≤ n (n = size of array), some elements appear twice and others appear once.

Find all the elements of [1, n] inclusive that do not appear in this array.

Could you do it without extra space and in O(n) runtime? You may assume the returned list does not count as extra space.

Example:

Input:
[4,3,2,7,8,2,3,1]

Output:
[5,6]

*/
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;


class Solution {
public:
    vector<int> findDisappearedNumbers(vector<int>& nums) {
        std::unordered_map<int, int> map;
        std::vector<int> res;
        for (const int num : nums)
        {
            map[num]++;
        }

        for (int i = 1; i <= nums.size(); ++i)
        {
            if (map[i] == 0) res.push_back(i);
        }

        return res;
    }
};


int main()
{
   
    Solution sln;
    std::vector<int> nums1 = { 1,2,3,4,5};
    std::vector<int> nums2 = { 1, 1, 1, 4, 5 };
    std::vector<int> nums3 = { 5, 5, 5, 5, 5 };
    
    for (auto n : sln.findDisappearedNumbers(nums3))
    {
        std::cout << n << ' ';
    }
    


    int f;
    std::cin >> f;
    
    return 0;
}
