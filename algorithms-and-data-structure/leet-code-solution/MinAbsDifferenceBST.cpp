/*
Minimum Absolute Difference in BST
URL: https://leetcode.com/problems/minimum-absolute-difference-in-bst/#/description

Given a binary search tree with non-negative values, find the minimum absolute difference between values of any two nodes.

Example:

Input:

    1
     \
      3
     /
    2

Output:
1

Explanation:
The minimum absolute difference is 1, which is the difference between 2 and 1 (or between 2 and 3).

Note: There are at least two nodes in this BST.


*/
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;


class Solution {

public:
    struct TreeNode {
        int val;
        TreeNode *left;
        TreeNode *right;
        TreeNode(int x) : val(x), left(NULL), right(NULL) {}
    };

    int getMinimumDifference(TreeNode* root) {
        if (!root) return min;

        getMinimumDifference(root->left);
        if (prev != -1)
        {
            min = std::min(min, root->val - prev);
        }

        prev = root->val;

        getMinimumDifference(root->right);

        return min;

    }

private:
    int min = 99999;
    int prev = -1;
};


int main()
{
   
   
    Solution sln;
   
    int f;
    std::cin >> f;
    
    return 0;
}
