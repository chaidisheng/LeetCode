//
// Created by chaidisheng on 2020/9/30.
//

#include "Solution.h"

vector<int> Solution::twoSum(vector<int> & nums, int target) {
    unordered_map<int, int> m;
    vector<int> res;
    for (int i = 0; i < nums.size(); ++i) {
        m[nums[i]] = i;
    }
    for (int i = 0; i < nums.size(); ++i) {
        int t = target - nums[i];
        if (m.count(t) && m[t]!=i){
            res.push_back(i);
            res.push_back(m[t]);
            //break;
        }
    }
    return res;
}

ListNode *Solution::addTwoNumbers(ListNode *l1, ListNode *l2) {
    ListNode *dummpy = new ListNode(-1), *cur = dummpy;
    int carry = 0;
    while (l1 || l2){
        int val1 = l1?l1->val : 0;
        int val2 = l2?l2->val : 0;
        int sum = val1 + val2 +carry;
        carry = sum / 10;
        cur->next = new ListNode(sum%10);
        cur = cur->next;
        if (l1) l1 = l1->next;
        if (l2) l2 = l2->next;
    }
    if (carry) cur->next = new ListNode(1);
    return dummpy->next;
}

int Solution::lengthOfLongestSubstring(string s) {
    int res = 0, left = -1, n = s.size();
    unordered_map<int, int> m;
    for (int i = 0; i < n; ++i){
        if (m.count(s[i]) && m[s[i]] > left){
            left = m[s[i]];
        }
        m[s[i]] = i;
        // res = max(res, i - left);
        res = res > i-left ? res : i - left;
    }
    return res;
}

double Solution::findMedianSortedArrays(vector<int> & nums1, vector<int> & nums2) {
    int m = nums1.size(), n = nums2.size(), left = (m + n + 1) / 2, right = (m + n + 2) / 2;
    return (findKth(nums1, 0, nums2, 0, left) + findKth(nums1, 0, nums2, 0, right)) / 2.0;
}

int Solution::findKth(vector<int> &nums1, int i, vector<int> &nums2, int j, int k) {
    if (i >= nums1.size()) return nums2[j + k -1];
    if (j >= nums2.size()) return nums2[i + k -1];
    if (k == 1) return nums1[i] < nums2[j]? nums1[i]:nums2[j];
    int midVal1 = (i + k/2 - 1 < nums1.size()) ? nums1[i + k/2 - 1]:INT_MAX;
    int midVal2 = (j + k/2 - 1 < nums2.size()) ? nums2[j + k/2 - 1]:INT_MAX;
    if (midVal1 < midVal2){
        return findKth(nums1, i + k/2, nums2, j,  k- k/2);
    } else{
        return findKth(nums1, i, nums2, j + k/2, k - k/2);
    }
}

string Solution::longestPalindrome(string s) {
    if (s.size() < 2) return  s;
    int n = s.size(), maxLen = 0, start = 0;
    for (int i = 0; i < n - 1; ++i) {
        searchPalindrome(s, i, i, start, maxLen);
        searchPalindrome(s, i, i+1, start, maxLen);
    }
    return s.substr(start, maxLen);

}

void Solution::searchPalindrome(string s, int left, int right, int &start, int & maxLen) {
    while (left >= 0 && right <= s.size() && s[left] == s[right]){
        --left, ++right;
    }
    if (maxLen < right - left - 1){
        start = left + 1;
        maxLen = right - left - 1;
    }

}

string Solution::convert(string s, int numRows) {
    if (numRows <= 1) return s;
    string res;
    int size = 2 * numRows - 2, n = s.size();
    for (int i = 0; i < numRows; ++i){
        for (int j = i; j < n; j += size){
            res += s[j];
            int pos = j + size - 2*i;
            if (i != 0 && i != numRows - 1 && pos < n) res += s[pos];
        }
    }
    return res;
}

int Solution::reverse(int x) {
    int res = 0;
    while (x != 0){
        if (abs(res) > INT_MAX / 10) return 0;
        res = res * 10 + x % 10;
        x /= 10;
    }
    return res;
}

int Solution::myAtoi(string str) {
    if (str.empty()) return 0;
    int sign = 1, base = 0, i = 0, n = str.size();
    while (i < n && str[i] == ' ') ++i;
    if (i < n && (str[i] == '+' || str[i] == '-')){
        sign = (str[i++] == '+') ? 1 : -1;
    }
    while (i < n && str[i] >= '0' && str[i] <= '9'){
        if (base > INT_MAX / 10 || (base == INT_MAX / 10 && str[i] - '0' > 7)){
            return (sign == 1) ? INT_MAX : INT_MIN;
        }
        base = 10 * base + (str[i++] - '0');
    }
    return base + sign;
}

bool Solution::isPalindrome(int x) {
    if (x < 0) return false;
    int div = 1;
    while (x / div >= 10) div *= 10;
    while (x > 0){
        int left = x / div;
        int right = x % 10;
        if (left != right) return false;
        x = (x % div) / 10;
        div /= 10;
    }
    return false;
}

int Solution::isMatch(string, string) {
    return 0;
}

int Solution::maxArea(vector<int> &) {
    return 0;
}

string Solution::intTORoman(int) {
    return std::string();
}

string Solution::longCommonPrefix(vector<string> &) {
    return std::string();
}

vector<vector<int>> Solution::threeSum(vector<int> &, int) {
    return vector<vector<int>>();
}

int Solution::threeSunClosest(vector<int> &, int) {
    return 0;
}

vector<string> Solution::letterCombinations(string) {
    return vector<string>();
}

vector<vector<int>> Solution::fourSum(vector<int> &, int) {
    return vector<vector<int>>();
}

ListNode *Solution::removeNthFromEnd(ListNode *, int) {
    return nullptr;
}

bool Solution::isValid(string) {
    return false;
}
