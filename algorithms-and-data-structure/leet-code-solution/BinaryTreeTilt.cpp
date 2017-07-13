/*
Binary Tree Tilt
URL: https://leetcode.com/problems/binary-tree-tilt/#/description

Given a binary tree, return the tilt of the whole tree.

The tilt of a tree node is defined as the absolute difference between the sum of all left subtree node values and the sum of all right subtree node values. Null node has tilt 0.

The tilt of the whole tree is defined as the sum of all nodes' tilt.

Example:

Input:
    1
  /   \
 2     3
Output: 1
Explanation:
Tilt of node 2 : 0
Tilt of node 3 : 0
Tilt of node 1 : |2-3| = 1
Tilt of binary tree : 0 + 0 + 1 = 1

Note:

The sum of node values in any subtree won't exceed the range of 32-bit integer.
All the tilt values won't exceed the range of 32-bit integer.




*/
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>

using namespace std;


class Solution {

public:
    //Definition for a binary tree node.
    struct TreeNode {
        int val;
        TreeNode *left;
        TreeNode *right;
        TreeNode(int x) : val(x), left(NULL), right(NULL) {}
        
    };
    
    int findTilt(TreeNode* root) {
        if (!root) return 0;

        int tilt = std::abs(sum(root->left) - sum(root->right));
        return tilt + findTilt(root->left) + findTilt(root->right);
    }

    

private:

    int sum(TreeNode* root)
    {
        return root->val + sum(root->left) + sum(root->right);
    }
   
};
