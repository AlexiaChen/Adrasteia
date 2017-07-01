/*
Detect Capital
URL: https://leetcode.com/problems/detect-capital/#/description

Given a word, you need to judge whether the usage of capitals in it is right or not.

We define the usage of capitals in a word to be right when one of the following cases holds:

All letters in this word are capitals, like "USA".
All letters in this word are not capitals, like "leetcode".
Only the first letter in this word is capital if it has more than one letter, like "Google".

Otherwise, we define that this word doesn't use capitals in a right way.

Example 1:

Input: "USA"
Output: True

Example 2:

Input: "FlaG"
Output: False

Note: The input will be a non-empty word consisting of uppercase and lowercase latin letters.
*/
#include <iostream>
#include <vector>
#include <string>
#include <cctype>

using namespace std;


class Solution {
public:
    bool detectCapitalUse(string word) {
       
        if (isAllUpper(word) || isAllLower(word))
        {
            return true;
        }

        if (std::isupper(word[0]) && isAllLower(std::string(word, 1, word.size() - 1)))
        {
            return true;
        }

        return false;
        
    }

private:
    bool isAllUpper(const std::string& str)
    {
        for (char c : str)
        {
            if (std::islower(c)) return false;
        }
        return true;
    }

    bool isAllLower(const std::string& str)
    {
        for (char c : str)
        {
            if (std::isupper(c)) return false;
        }
        return true;
    }
};


int main()
{
   
    Solution sln;
    std::cout << sln.detectCapitalUse("Google") << sln.detectCapitalUse("USA") << 
        sln.detectCapitalUse("FlaG") << sln.detectCapitalUse("g");

    int f;
    std::cin >> f;
    
    return 0;
}
