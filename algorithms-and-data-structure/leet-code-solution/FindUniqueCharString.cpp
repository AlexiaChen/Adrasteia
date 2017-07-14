/*
First Unique Character in a String
URL: https://leetcode.com/problems/first-unique-character-in-a-string/#/description

Given a string, find the first non-repeating character in it and return it's index. If it doesn't exist, return -1.

Examples:

s = "leetcode"
return 0.

s = "loveleetcode",
return 2.

Note: You may assume the string contain only lowercase letters.

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
    int firstUniqChar(string s) {
        std::unordered_map<char, int> alphaMap(26);
        for (char c : s) alphaMap[c]++;
        for (int i = 0; i < s.size(); ++i)
        {
            if (alphaMap[s[i]] == 1) return i;
        }
        return -1;
    }
private:
     
};
