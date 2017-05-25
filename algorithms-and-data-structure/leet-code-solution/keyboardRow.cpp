/*
Keyboard Row
URL: https://leetcode.com/problems/keyboard-row/#/description

Example 1:

Input: ["Hello", "Alaska", "Dad", "Peace"]
Output: ["Alaska", "Dad"]

Note:

You may use one character in the keyboard more than once.
You may assume the input string will only contain letters of alphabet.

*/
#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>

using namespace std;

class Solution {
public:
    vector<string> findWords(vector<string>& words) {
        static const std::unordered_set<char> row1 = { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p' };
        static const std::unordered_set<char> row2 = { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'};
        static const std::unordered_set<char> row3 = { 'z', 'x', 'c', 'v', 'b', 'n', 'm'};
        static const std::vector<std::unordered_set<char>> alphabet_table = {row1,row2,row3};
        std::vector<std::string> r;
        for (auto& word : words)
        {
            if (checkWord(word,alphabet_table)) r.push_back(word);
        }
        return r;
    }
private:
    bool checkWord(const std::string &word, const std::vector<std::unordered_set<char>>& lookup)
    {
       
        bool r1 = checkWordForRow(word, lookup[0]); 
        bool r2 = checkWordForRow(word, lookup[1]);
        bool r3 = checkWordForRow(word, lookup[2]);
        
        if (r1 || r2 || r3) return true;
        else return false;

    }

    bool checkWordForRow(const std::string& word, const std::unordered_set<char>& row)
    {
        bool ret = true;
        std::string lower_word(word);
        std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), tolower);
        for (char c : lower_word)
        {
            if (row.find(c) == row.end())
            {
                ret = false;
                break;
            }
        }
        return ret;
    }
};

int main()
{
    
    std::vector<std::string> words = { "Hello", "Alaska", "Dad", "Peace","Gas","We","Fake" };
    std::vector<std::string> r;
    Solution sln;
    r = sln.findWords(words);
    for each (std::string str in r)
    {
        std::cout << str << " ";
    }
 
    int d;
    std::cin >> d;
    
    return 0;
}