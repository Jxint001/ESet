#include "src.hpp"
// #if defined (_UNORDERED_MAP_)  || (defined (_LIST_)) || (defined (_MAP_)) || (defined (_SET_)) || (defined (_UNORDERED_SET_))||(defined (_GLIBCXX_MAP)) || (defined (_GLIBCXX_UNORDERED_MAP))
// BOOM :)
// #endif
#include<iostream>
void flag() {std::cout << "ok" << std::endl; }
int main() {
    freopen("1.in", "r", stdin);
    freopen("1.out", "w", stdout);
//    srand(0);
//    clock_t start, end;
//    start = clock();
    ESet<long long> s[25];
    ESet<long long>::iterator it;
    
    int op, lst=0, it_a=-1, valid = 0, cnt=1;
    while (scanf("%d", &op) != EOF) {
        //flag();
        long long a, b, c;
//        if(cnt==213){
//            printf("!!!");
//        }
        switch (op) {
            case 0: {
                scanf("%lld%lld", &a, &b);
                //std::cout << "in case 0" << std::endl;
                auto p=s[a].emplace(b);
                //std::cout << "case 0 emplace" << std::endl;
                if(p.second) {
                	it_a = a;
                    it = p.first;
                    valid = 1;
                }
                break;
            }
            case 1:
                scanf("%lld%lld", &a, &b);
                if (valid && it_a==a && *it == b)valid = 0;
                s[a].erase(b);
                break;
            case 2:
                scanf("%lld", &a);
                s[++lst] = s[a];
                break;
            case 3: {
                scanf("%lld%lld", &a, &b);
                auto it2 = s[a].find(b);
                if (it2 != s[a].end()) {
                    printf("true\n");
                    it_a = a;
                    it = it2;
                    valid = 1;
                } else
                    printf("false\n");
                cnt++;
                break;
            }
            case 4:
                scanf("%lld%lld%lld", &a, &b, &c);
                //printf("%d\n", s[a].range(b, c));
                //std::cout << "range" << std::endl;
                printf("%ld\n", s[a].range(b, c));
                cnt++;
                break;
            case 5:
            //std::cout << "in case 5" << std::endl;
                if (valid){
                    auto it2=it;
                    if (it==--it2)valid = 0;
                    //flag();
                }
                if (valid)
                    printf("%lld\n", *(--it));
                else
                    printf("-1\n");
                cnt++;
                break;
            case 6:
                if (valid) {
                    auto it2=++it;
                    if (it==++it2)valid = 0;
                    else printf("%lld\n", *it);
                }
                if (!valid)
                    printf("-1\n");
                cnt++;
                break;
        }
    }
    auto y = s[2].begin();
    for (auto it = s[2].begin(); it != s[2].end(); ++it) {
        if (it != s[2].begin()) {
            if (!(*y < *it)) {
                std::cout << "fuck" << std::endl;
            }
            ++y;
        }
        int siz0 = it.it->son[0] ? it.it->son[0]->siz : 0;
        int siz1 = it.it->son[1] ? it.it->son[1]->siz : 0;
        if (it.it->siz != siz0 + siz1 + 1) {std::cout << "shit" << std::endl; }
    }
    // 检查每个链表节点it在树上find(it->key)后指针是否一致
    for (auto it = s[0].begin(); it != s[0].end(); ++it) {
        auto tree_ptr = s[0].find(*it).it;
        if (tree_ptr != it.it) {
            std::cout << "tree/linked-list out of sync for key=" << *it << std::endl;
        }
    }
//    end = clock(); //程序结束用时
//    double endtime = (double) (end - start) / CLOCKS_PER_SEC;
//    printf("time=%lf s\n", endtime);
    return 0;
}
