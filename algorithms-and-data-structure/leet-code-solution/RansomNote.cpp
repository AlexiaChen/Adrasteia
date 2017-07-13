/*
Ransom Note
URL: https://leetcode.com/problems/ransom-note/#/description

Given an arbitrary ransom note string and another string containing letters from all the magazines, write a function that will return true if the ransom note can be constructed from the magazines ; otherwise, it will return false.

Each letter in the magazine string can only be used once in your ransom note.

Note:
You may assume that both strings contain only lowercase letters.

canConstruct("a", "b") -> false
canConstruct("aa", "ab") -> false
canConstruct("aa", "aab") -> true



*/
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>

using namespace std;


class Solution {

public:
    bool canConstruct(string ransomNote, string magazine) {
        std::unordered_map<char, int> map(26);
        for (char c : magazine) map[c]++;
        for (char c : ransomNote)
        {
            map[c]--;
            if (map[c] < 0) return false;
        }
        return true;
    }

    

private:
   
};


int main()
{
   
   
    Solution sln;

    std::cout << sln.canConstruct("a", "b") << std::endl;
    std::cout << sln.canConstruct("aa", "ab") << std::endl;
    std::cout << sln.canConstruct("aa", "aab") << std::endl;
    std::cout << sln.canConstruct("aa", "abafsa") << std::endl;

   
    int f;
    std::cin >> f;
    
    return 0;
}
