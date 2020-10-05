#include <iostream>
#include "Solution.h"
#include "List.h"

int main() {

    Solution objective;
    vector<int> res;
    vector<int> nums;
    nums.push_back(3);
    nums.push_back(7);
    nums.push_back(4);
    nums.push_back(5);
    nums.push_back(2);
    nums.push_back(2);
    res = objective.twoSum(nums, 9);
    for(const int& k : nums)
        cout << k << " ";
    cout << endl;
    for(const int& k : res)
        cout << k << " ";
    cout << endl;
    std::cout << "Hello, World!" << std::endl;

    string s = "pwwkewabbdjhjhjkafhjkahjh";
    cout << objective.lengthOfLongestSubstring(s) << endl;

    vector<int> nums1;
    nums1.push_back(1);
    nums1.push_back(3);
    vector<int> nums2;
    nums2.push_back(2);

    std::cout << objective.findMedianSortedArrays(nums1, nums2) << std::endl;
    std::cout << objective.longestPalindrome(s) << std::endl;
    std::cout << objective.convert(s, 3) << std::endl;

    ListNode *l1 = new ListNode(5), *l2 = new ListNode(4);
    std::cout << objective.addTwoNumbers(l1, l2)->val << std::endl;
    std::cout << "Hello, World!" << std::endl;

    cout << "---------------------" << endl;
    List list;
    list.insert(30);
    list.insert(20);
    list.insert(10);
    list.insert_pos(10, 5);
    list.print();
    cout << "---------------------" << endl;
    list.erase(10);
    list.print();
    cout << "---------------------" << endl;
    list.reverse();
    list.print();
    cout << "---------------------" << endl;
    list.update(5, 8);
    list.print();

    return 0;
}
