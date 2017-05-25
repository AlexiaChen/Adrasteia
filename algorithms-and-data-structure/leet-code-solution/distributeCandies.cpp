/*
Distribute Candies
URL: https://leetcode.com/problems/distribute-candies/#/description

Example 1:

Input: candies = [1,1,2,2,3,3]
Output: 3
Explanation:
There are three different kinds of candies (1, 2 and 3), and two candies for each kind.
Optimal distribution: The sister has candies [1,2,3] and the brother has candies [1,2,3], too.
The sister has three different kinds of candies.

Example 2:

Input: candies = [1,1,2,3]
Output: 2
Explanation: For example, the sister has candies [2,3] and the brother has candies [1,1].
The sister has two different kinds of candies, the brother has only one kind of candies.

Note:

The length of the given array is in range [2, 10,000], and will be even.
The number in given array is in range [-100,000, 100,000].

*/
#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

using namespace std;

class Solution {
public:
    int distributeCandies(vector<int>& candies) {
        std::unordered_set<int> hash_set;
        const int half = candies.size() / 2;
        for (const int& value : candies)
        {
            hash_set.insert(value);
        }
        return std::min<int>(hash_set.size(), half);
    }
};

int main()
{
    
    std::vector<int> nums0 = {1,1,2,2,3,3};
    std::vector<int> nums1 = { 1, 1, 2, 3 };
    Solution sln;
    std::cout << sln.distributeCandies(nums0) << std::endl;
    std::cout << sln.distributeCandies(nums1) << std::endl;
    
    int d;
    std::cin >> d;
    
    return 0;
}