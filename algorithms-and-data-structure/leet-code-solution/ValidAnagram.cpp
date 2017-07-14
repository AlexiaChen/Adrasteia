/*
Valid Anagram
URL: https://leetcode.com/problems/valid-anagram/#/description

Given two strings s and t, write a function to determine if t is an anagram of s.

For example,
s = "anagram", t = "nagaram", return true.
s = "rat", t = "car", return false.

Note:
You may assume the string contains only lowercase alphabets.
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
    bool isAnagram(string s, string t) {
        if (s.size() != t.size()) return false;

        size_t size = s.size();
        std::unordered_map<char, int> counts(26);
        for (int i = 0; i < size; ++i)
        {
            counts[s[i]]++;
            counts[t[i]]--;
        }

        for (auto cnt : counts)
        {
            if (cnt.second) return false;
        }
        return true;
    }
    
private:
     
};
