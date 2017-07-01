/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */
class Solution {
public:
    int maxDepth(TreeNode* root) {
        if (!root) return 0;

        int max_left = 1 + maxDepth(root->left);
        int max_right = 1 + maxDepth(root->right);

        return max_left > max_right ? max_left : max_right;
    }
};
