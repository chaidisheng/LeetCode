//
// Created by chaidisheng on 2020/9/30.
//

#ifndef LEETCODE_SOLUTION_H
#define LEETCODE_SOLUTION_H
#include<vector>
#include <unordered_map>
using namespace std;

struct ListNode {
    int val;
    ListNode *next;
    explicit ListNode(const int& x) : val(x), next(nullptr) {}
};

class Solution {
public:
    vector<int> twoSum(vector<int>&, int);
    ListNode* addTwoNumbers(ListNode*, ListNode*);
    int lengthOfLongestSubstring(string s);
    double findMedianSortedArrays(vector<int>&, vector<int>&);
    int findKth(vector<int>&, int, vector<int>&, int, int);
    string longestPalindrome(string);
    void searchPalindrome(string, int, int, int&, int&);
    string convert(string, int);
    int reverse(int);
    int myAtoi(string);
    bool isPalindrome(int);

};

#endif //LEETCODE_SOLUTION_H
