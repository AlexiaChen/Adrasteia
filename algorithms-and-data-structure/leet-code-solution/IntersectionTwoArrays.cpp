/*
Intersection of Two Arrays
URL: https://leetcode.com/problems/intersection-of-two-arrays/#/description

Given two arrays, write a function to compute their intersection.

Example:
Given nums1 = [1, 2, 2, 1], nums2 = [2, 2], return [2].

Note:

Each element in the result must be unique.
The result can be in any order.


*/
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <iterator>
#include <set>

using namespace std;


class Solution {

public:
    vector<int> intersection(vector<int>& nums1, vector<int>& nums2) {
        std::vector<int> intersection_result;
        
        std::sort(nums1.begin(), nums1.end());
        std::sort(nums2.begin(), nums2.end());

        std::set_intersection(nums1.begin(), nums1.end(), 
                              nums2.begin(), nums2.end(), 
                              std::back_inserter(intersection_result));
        
        std::set<int> r_set;
        for (auto v : intersection_result) r_set.insert(v);

        intersection_result.resize(r_set.size());
        intersection_result.clear();
        std::copy(r_set.begin(), r_set.end(), std::back_inserter(intersection_result));
        return intersection_result;
    }
private:
   
};


int main()
{
   
   
    Solution sln;
    std::vector<int> num1{ 1, 2, 2, 1 };
    std::vector<int> num2{ 2,2};
    sln.intersection(num1, num2);
    int f;
    std::cin >> f;
    
    return 0;
}
