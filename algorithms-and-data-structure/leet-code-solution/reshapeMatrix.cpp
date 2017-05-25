/*
Reshape the Matrix
URL: https://leetcode.com/problems/reshape-the-matrix/#/description

Example 1:

Input:
nums =
[[1,2],
[3,4]]
r = 1, c = 4
Output:
[[1,2,3,4]]
Explanation:
The row-traversing of nums is [1,2,3,4]. The new reshaped matrix is a 1 * 4 matrix, fill it row by row by using the previous list.

Example 2:

Input:
nums =
[[1,2],
[3,4]]
r = 2, c = 4
Output:
[[1,2],
[3,4]]
Explanation:
There is no way to reshape a 2 * 2 matrix to a 2 * 4 matrix. So output the original matrix.

Note:

The height and width of the given matrix is in range [1, 100].
The given r and c are all positive.

*/
#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    vector<vector<int>> matrixReshape(vector<vector<int>>& nums, int r, int c) {
        if (nums.size() == 0) return nums;
        int origin_rows = nums.size();
        int origin_cols = nums[0].size();
        if (r*c > origin_cols*origin_rows) return nums;

        std::vector<std::vector<int>> result(r,std::vector<int>(c));
        std::vector<int> temp(origin_rows*origin_cols);
        for (int row = 0; row < origin_rows; ++row)
        {
            for (int col = 0; col < origin_cols; ++col)
            {
                int k = row * origin_cols + col;
                result[k/c][k%c] = nums[row][col];
            }
        }

        return result;
        
    }
};

int main()
{
    std::vector<int> row0 = { 1, 2 };
    std::vector<int> row1 = { 3, 4 };
    std::vector<std::vector<int>> nums;
    nums.push_back(row0);
    nums.push_back(row1);
    Solution sln;
    auto result = sln.matrixReshape(nums,1,4);
    
    int d;
    std::cin >> d;
    
    return 0;
}