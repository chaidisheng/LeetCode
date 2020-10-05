//
// Created by chaidisheng on 2020/9/30.
//

#include <iostream>
#include "List.h"

void List::create_List() {
    head = new ListNode(0);
}

void List::insert(const int &d) {
    ListNode *p = new ListNode(d);
    p->next = head->next;
    head->next = p;
}

void List::insert_pos(const int &d, const int &d1) {
    ListNode * p = find(d);
    ListNode * q = new ListNode(d1);
    q->next = p->next;
    p->next = q;
}

void List::erase(const int &d) {
    ListNode * p = find(d);
    ListNode *q = p->next;
    p->next = p->next->next;
    delete q;
}

void List::update(const int &d, const int &d1) {
    ListNode * p = find(d);
    p->next->val = d1;
}

void List::reverse() {
    ListNode * p = head->next;
    ListNode * q = head->next->next;
    ListNode * m = head->next->next->next;
    p->next = nullptr;
    while(m){
        q->next = p;
        p = q;
        q = m;
        m = m->next;
    }
    q->next = p;
    head ->next = q;
}

void List::print() {
    for(ListNode * p = head->next; p; p=p->next){
        std::cout << p->val << std::endl;
    }
}
