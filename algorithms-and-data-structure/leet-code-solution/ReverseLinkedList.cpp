/*
Reverse Linked List
URL: https://leetcode.com/problems/reverse-linked-list/description/

Reverse a singly linked list.

Hint:
A linked list can be reversed either iteratively or recursively. Could you implement both?

*/              
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <cmath>
#include <string>
#include <unordered_set>

using namespace std;



class Solution {

public:

    struct ListNode {
        int val;
        ListNode *next;
        ListNode(int x) : val(x), next(NULL) {}
    };
    //  1 -> 2 -> 3 -> 4 -> 5 -> 6
    ListNode* reverseList(ListNode* head) {
        ListNode* pre = NULL;
        while (head)
        {
            ListNode* current_next = head->next;
            head->next = pre;
            pre = head;
            head = current_next;
        }
        return pre;
    }
   
};


int main()
{
   
   
    Solution sln;

   
    int f;
    std::cin >> f;
    
    return 0;
}
