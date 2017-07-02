/*
Invert Binary Tree
URL: https://leetcode.com/problems/invert-binary-tree/#/description

Invert a binary tree.

4                      
/   \
2     7
/ \   / \
1   3 6   9

to

4
/   \
7     2
/ \   / \
9   6 3   1

Trivia:
This problem was inspired by this original tweet by Max Howell:

Google: 90% of our engineers use the software you wrote (Homebrew), but you can’t invert a binary tree on a whiteboard so fuck off.
*/
#include <iostream>
#include <unordered_map>

using namespace std;


class Solution {
public:
    struct TreeNode {
        int val;
        TreeNode *left;
        TreeNode *right;
        TreeNode(int x) : val(x), left(NULL), right(NULL) {}
        
    };
public:
    TreeNode* invertTree(TreeNode* root) {
        if (!root) return NULL;

        swapNode(root->left, root->right);
        root->left = invertTree(root->left);
        root->right = invertTree(root->right);

        return root;
    }
private:
    void swapNode(TreeNode* &left, TreeNode* &right)
    {
        TreeNode* temp;
        temp = left;
        left = right;
        right = temp;
    }
};
