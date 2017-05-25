/*
Island Perimeter
URL: https://leetcode.com/problems/island-perimeter/#/description

The grid is rectangular, width and height don't exceed 100. Determine the perimeter of the island.

Example:

[[0,1,0,0],
[1,1,1,0],
[0,1,0,0],
[1,1,0,0]]

Answer: 16
Explanation: The perimeter is the 16 yellow stripes in the image below:
*/
#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    int islandPerimeter(vector<vector<int>>& grid) {
        int island_count = 0;
        int island_side_count = 0;
        for (int i = 0; i < grid.size(); ++i)
        {
            for (int j = 0; j < grid[0].size(); ++j)
            {
                if (grid[i][j] == 1)
                {
                    island_count++;
                    if (i < grid.size() - 1 && grid[i + 1][j] == 1) island_side_count++; // down side
                    if (j < grid[0].size() - 1 && grid[i][j + 1] == 1) island_side_count++; //right side;
                }
            }
        }
        return 4*island_count - island_side_count*2;
    }

};

int main()
{
   
    Solution sln;
    std::vector<std::vector<int>> grid = {
        { 0, 1, 0, 0 },
        { 1, 1, 1, 0 },
        { 0, 1, 0, 0 },
        { 1, 1, 0, 0 }
    
    };
    std::vector<std::vector<int>> grid1 = {
        {1}

    };
    std::cout << sln.islandPerimeter(grid) << std::endl;
    int d;
    std::cin >> d;
    
    return 0;
}