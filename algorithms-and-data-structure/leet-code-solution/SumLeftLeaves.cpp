/*
Sum of Left Leaves
URL: https://leetcode.com/problems/sum-of-left-leaves/#/description

Find the sum of all left leaves in a given binary tree.

Example:

      3
     / \
    9  20
      /  \
     15   7

There are two left leaves in the binary tree, with values 9 and 15 respectively. Return 24.


*/
#include <iostream>
#include <algorithm>
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
    
    int sumOfLeftLeaves(TreeNode* root) {
        if (!root) return 0;
        // 从左叶子节点的父节点的角度来判断是否是左叶子节点，否则左叶子节点不能判断自身是左叶子节点
        if (root->left && !root->left->left && !root->left->right) return root->left->val + sumOfLeftLeaves(root->right);
        return sumOfLeftLeaves(root->left) + sumOfLeftLeaves(root->right);
    }
   
};
