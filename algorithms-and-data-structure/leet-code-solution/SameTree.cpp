/*
Same Tree
URL: https://leetcode.com/problems/same-tree/#/description

Given two binary trees, write a function to check if they are equal or not.

Two binary trees are considered equal if they are structurally identical and the nodes have the same value.

[10,5,15]
[10,5,null,null,15]

false

    10          10
    / \        /  \
   5  15      5
               \
               15
*/              
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <cmath>
#include <string>

using namespace std;


class Solution {

public:
    struct TreeNode
    {
        int val;
        TreeNode *left;
        TreeNode *right;
        TreeNode(int x) : val(x), left(NULL), right(NULL) {}
    };
    bool isSameTree(TreeNode* p, TreeNode* q) {
        if (!p && !q) return true;
        if (p && !q) return false;
        if (!p && q) return false;
        if (p && q)
        {
            bool ret = p->val == q->val ? true : false;
            return ret && isSameTree(p->left, q->left) && isSameTree(p->right, q->right);
        }
        
    }
private:
     
};
