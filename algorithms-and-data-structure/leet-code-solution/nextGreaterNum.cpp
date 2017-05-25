/*
Next Greater Element I
URL: https://leetcode.com/problems/next-greater-element-i/#/description

Example 1:

Input: nums1 = [4,1,2], nums2 = [1,3,4,2].
Output: [-1,3,-1]
Explanation:
For number 4 in the first array, you cannot find the next greater number for it in the second array, so output -1.
For number 1 in the first array, the next greater number for it in the second array is 3.
For number 2 in the first array, there is no next greater number for it in the second array, so output -1.

Example 2:

Input: nums1 = [2,4], nums2 = [1,2,3,4].
Output: [3,-1]
Explanation:
For number 2 in the first array, the next greater number for it in the second array is 3.
For number 4 in the first array, there is no next greater number for it in the second array, so output -1.

Note:

All elements in nums1 and nums2 are unique.
The length of both nums1 and nums2 would not exceed 1000.

*/
#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    vector<int> nextGreaterElement(vector<int>& findNums, vector<int>& nums) {
        std::vector<int> r;
        for (auto v : findNums)
        {
            r.push_back(findNextGreaterNum(nums, v));
        }
        return r;
    }

private:
    int findNextGreaterNum(std::vector<int> &vec,int v)
    {
        int findIndex;
        for (int i = 0; i < vec.size(); ++i)
        {
            if (v == vec[i])
            {
                findIndex = i;
                break;
            }
        }

        int findNextIndex = findIndex + 1;
        if (findNextIndex >= vec.size()) return -1;
        for (int i = findNextIndex; i < vec.size(); ++i)
        {
            if (v < vec[i])
            {
                return vec[i];
            }
        }
        return -1;
    }


};
int main()
{
   
    Solution sln;
    std::vector<int> num1 = { 2,4};
    std::vector<int> num2 = {1,2,3,4};
    std::vector<int> r;
    r = sln.nextGreaterElement(num1, num2);
    for each  (int i in r)
    {
        std::cout << i << " ";
    }
    
    int d;
    std::cin >> d;
    
    return 0;
}