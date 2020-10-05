//
// Created by chaidisheng on 2020/9/30.
//

#ifndef LEETCODE_LIST_H
#define LEETCODE_LIST_H

class List {
private:
    struct ListNode {
        int val;
        ListNode *next;
        explicit ListNode(const int& x) : val(x), next(nullptr) {}
    };
    ListNode *head{};
    void clear(){
        ListNode * p = head;
        while(p){
            ListNode * q = p->next;
            delete p;
            p = q;
        }
    }

    ListNode* find(const int& d){
        ListNode * p = head;
        for(; p; p=p->next){
            if(p->next->val==d)
                break;
        }
        return p;
    }

public:
    List(){create_List();}
    ~List(){clear();}
    void create_List();
    void insert(const int& d);
    void insert_pos(const int& d,const int& d1);
    void erase(const int& d);
    void update(const int& d,const int& d1);
    void reverse();
    void print();
};


#endif //LEETCODE_LIST_H
