//
// Created by chaidisheng on 2020/9/30.
//

#ifndef LEETCODE_SOLUTION_H
#define LEETCODE_SOLUTION_H
#include<vector>
#include <unordered_map>
#include<climits>
#include <cmath>
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
    int isMatch(string, string);
    int maxArea(vector<int>&);
    string intTORoman(int);
    string longCommonPrefix(vector<string>&);
    vector<vector<int>> threeSum(vector<int>&, int);
    int threeSunClosest(vector<int>&, int);
    vector<string> letterCombinations(string);
    vector<vector<int>> fourSum(vector<int>&, int);
    ListNode* removeNthFromEnd(ListNode*, int);
    bool isValid(string);



};

#endif //LEETCODE_SOLUTION_H
