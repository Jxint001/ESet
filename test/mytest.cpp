#include "src.hpp"
#include <cassert>
#include <iostream>
#include <set>
using std::cout, std::endl;
int main() {
    // ESet<int> set;
    // for (int i = 0; i < 100; ++i) {
    //     //cout << i << endl;
    //     set.emplace(i);
    // }
    // int i = 0;
    // for (auto it = set.begin(); it != set.end(); ++it) {
    //     if (*it != i++) {cout << "fuck" << endl; }
    // }
    // cout << set.size() << endl;

    // for (int i = 0; i < 56; i += 4) {
    //     //cout << i << endl;
    //     set.erase(i);
    // }
    // // int t = 56;
    // // for (auto it = set.begin(); it != set.end(); ++it) {
    // //     if (*it != t++) {cout << "fuck" << endl; }
    // // }
    // // cout << set.size() << endl;

    // // auto it = set.end();
    // // cout << *(--it) << endl;
    // // auto it2 = it;
    // // cout << (it == --it2) << endl;

    // // it = set.begin();
    // // it2 = it;
    // // cout << (it == --it2) << endl;

    // // cout << set.range(0, 0) << endl;
    // // cout << *set.upper_bound(11) << endl;

    // int cnt = 0;
    // for (auto it = set.begin(); it != set.end(); ++it) {
       
    //     int siz0 = it.it!=nullptr && it.it->son[0]!=nullptr ? it.it->son[0]->siz : 0;
    //     int siz1 = it.it!=nullptr && it.it->son[1]!=nullptr ? it.it->son[1]->siz : 0;

    //     if (it.it->siz != siz0 + siz1 + 1) {std::cout << "shit" << std::endl; cnt++;}
    // }
    // cout << cnt << endl;

    // //cout << set.range(1, 2) << endl;

    // // set.loop_print();
    // // std::cout << std::endl;
    // // set.print(set.root);
    // // std::cout << std::endl;

    // ESet<int> s;
    // s.emplace(1);
    // s.emplace(2);
    // s.emplace(3);
    // s.erase(2);

    // ESet<int> r;
    // // for (int i = 0; i < 1000000; ++i) {
    // //     r.emplace(i);
    // // }
    // // for (int j = 3; j < 235; j += 7) {
    // //     r.erase(j);
    // // }
    // r.emplace(3);r.emplace(1);
    // auto key = r.root->key;
    // cout << (r.root->color == BLACK) << endl;
    // cout << r.erase(key) << endl;

    // std::set<int> stdset;
    // auto sit = stdset.emplace(1).first;
    // for (int i = 0; i <= 2; i++) {
    //     stdset.emplace(i);
    // }
    // while (sit != stdset.end()) {
    //     stdset.erase(*(sit++));
    // }
    // cout << "size: " << stdset.size() << endl;
    // cout << "---" << endl;
    ESet<int> s2;
    auto it = s2.emplace(1).first;
    for (int i = 0; i <= 2; i++) {
        s2.emplace(i);
    }
    cout << "*it: " << *(it) << endl;
   // cout << "key: " << (s2.root->key) << endl;
    bool flg = false;
    int t = 0;
    while (it != s2.end()) {
        ++t;
        if (flg) {cout << "iterate after over" << endl;}
        s2.erase(*(it++));
        if (t == 1) {
            cout << "t is 1" << endl;
            //if (it != ESet<int>::iterator(s2.root)) {cout << "noooo" << endl;}
            //if (it.it->next != nullptr)  cout << "causing leak" << endl;
        }
        if (it == s2.end()) {cout << "over" << endl; flg = true;}
    }
    cout << "hi" << endl;
    cout << "mysize: " << s2.size() << endl;

    cout << "---" << endl;
    ESet<int> set;
    for (int i = 0; i < 100; ++i) {
        set.emplace(i);
    }
    for (int i = 0; i < 10; i += 1) {
        set.erase(i);
    }
    cout << set.range(0, 99) << endl;
    cout << *(set.begin()) << endl;
    return 0;
}