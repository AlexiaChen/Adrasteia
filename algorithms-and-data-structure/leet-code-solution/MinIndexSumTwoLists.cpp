/*
Minimum Index Sum of Two Lists
URL: https://leetcode.com/problems/minimum-index-sum-of-two-lists/#/description

Suppose Andy and Doris want to choose a restaurant for dinner, and they both have a list of favorite restaurants represented by strings.

You need to help them find out their common interest with the least list index sum. If there is a choice tie between answers, output all of them with no order requirement. You could assume there always exists an answer.

Example 1:

Input:
["Shogun", "Tapioca Express", "Burger King", "KFC"]
["Piatti", "The Grill at Torrey Pines", "Hungry Hunter Steakhouse", "Shogun"]
Output: ["Shogun"]
Explanation: The only restaurant they both like is "Shogun".

Example 2:

Input:
["Shogun", "Tapioca Express", "Burger King", "KFC"]
["KFC", "Shogun", "Burger King"]
Output: ["Shogun"]
Explanation: The restaurant they both like and have the least index sum is "Shogun" with index sum 1 (0+1).

Note:

The length of both lists will be in the range of [1, 1000].
The length of strings in both lists will be in the range of [1, 30].
The index is starting from 0 to the list length minus 1.
No duplicates in both lists.


*/
#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cmath>

using namespace std;


class Solution {

public:
    vector<string> findRestaurant(vector<string>& list1, vector<string>& list2) {
        std::unordered_map<std::string, std::pair<int,bool>> lookup_tab;
        std::vector<std::pair<int, int>> index_array;
        std::unordered_map<int, std::vector<std::pair<int, int>>> min_sum_lookup;
        std::vector<int> index_sum_array;
       
        
        for (int i = 0; i < list1.size(); ++i) lookup_tab[list1[i]] = std::make_pair(i,true);

        for (int j = 0; j < list2.size(); ++j)
        {
            if (lookup_tab[list2[j]].second)
            {
                int list1_index = lookup_tab[list2[j]].first;
                int list2_index = j;
                int index_sum = list1_index + list2_index;
                
                index_array.push_back(std::make_pair(list1_index, list2_index));
                min_sum_lookup[index_sum].push_back(std::make_pair(list1_index,list2_index));
                index_sum_array.push_back(index_sum);
            }
        }

        std::sort(index_sum_array.begin(), index_sum_array.end(),std::less<int>());

        std::vector<std::string> ret;
        int min_sum = index_sum_array[0];
        for (auto v : min_sum_lookup[min_sum])
        {
            int i = v.first;
            ret.push_back(list1[i]);
        }

        return ret;
    }
private:
   
};
