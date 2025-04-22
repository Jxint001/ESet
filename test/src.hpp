#include <queue>
 #include <iostream>
#include <utility>
#include <vector>
//void hi() {std::cout << "hello" << std::endl; }
// #include <unordered_set>
enum Color {BLACK, RED};
template <class Key, class Compare = std::less<Key>>
class ESet {
    public:
    struct Node{
        Node *son[2], *parent, *prev, *next;
        Color color;
        Key key;
        size_t siz;
        Node() : key(Key()), son{nullptr, nullptr}, parent(nullptr), prev(nullptr), next(nullptr), color(RED), siz(1) {}
        Node(const Key& k) : key(k), son{nullptr, nullptr}, parent(nullptr), prev(nullptr), next(nullptr), color(RED), siz(1) {}
    };
    Node *root = nullptr;
    Node *first = nullptr;
    Node *tail = nullptr;
    size_t count = 0;
    Compare comp;
    std::vector<Node*> pool;

    Node* NEW_NODE(const Key& key) {
        if (!pool.empty()) {
            Node *node = pool.back();  pool.pop_back();
            *node = Node(key);
            return node;
        }
        return new Node(key);
    }

    void RECYCLE(Node* node) {
        pool.push_back(node);
    }

    void UpdSiz(Node *u) {
        u->siz = 1;
        if (u->son[0]) {u->siz += u->son[0]->siz; }
        if (u->son[1]) {u->siz += u->son[1]->siz; }
    }

    void clear() {
        Node *cur = first, *nxt;
        int t = 0;
        while (cur != nullptr) {
            nxt = cur->next;
            //delete cur;
            RECYCLE(cur);
            cur = nxt;
        }
        //std::cout << "ok" << std::endl;
        root = nullptr;  first = nullptr;  tail = nullptr;
        count = 0;
    }

    // 以u为旋转点的旋转，flg为0是左旋，否则右旋
    // 注意旋转操作不影响prev和next，不影响color，影响son,parent和siz
    void Rotate(Node *u, bool flg) {
        Node *k = u->son[flg ^ 1];
        Node *move = k->son[flg];
        u->son[flg ^ 1] = move;
        if (move != nullptr)  move->parent = u;
        k->parent = u->parent;
        if (u->parent != nullptr)  u->parent->son[u == u->parent->son[1]] = k;
        else root = k;
        k->son[flg] = u;
        u->parent = k;
        UpdSiz(u);  UpdSiz(k);
    }

    void Del_in_list(Node *u) {
        Node *nxt = u->next, *pre = u->prev;
        if (pre != nullptr)  {
            pre->next = nxt;}
        if (nxt != nullptr)  nxt->prev = pre;
        if (u == first)  first = nxt;
        if (u == tail)  tail = pre;
    }
    void Del_in_tree(Node *u) {
        if (u->parent != nullptr) {
            u->parent->son[u == u->parent->son[1]] = nullptr;
            Node *cur = u->parent;
            while (cur != nullptr) {
                UpdSiz(cur);
                cur = cur->parent;
            }
        } else {
            root = nullptr;
        }
    }
    //直接删除某个叶节点涉及到son, parent, siz, prev, next所有，以及可能有first
    // 这里没处理count
    void DeleteLeaf(Node *u) {
        // 先把链表结构修改
        Del_in_list(u);
        // 再修改树结构
        Del_in_tree(u);
        //delete u;
        RECYCLE(u);
    }

    void copy_subtree(Node* node) {
        if (!node) return;
        copy_subtree(node->son[0]);
        emplace(node->key);
        copy_subtree(node->son[1]);
    }

    void swap_nodes(Node* z, Node* y) {
        // y == z->next && z == y->prev
        Node* zp = z->prev;
        Node* yn = y->next;
        if (zp) zp->next = y;  else first = y;
        if (yn) yn->prev = z;
        y->prev = zp;  y->next = z;
        z->prev = y;   z->next = yn;

        if (z == tail)  tail = y;
        else if (y == tail)  tail = z;

        Node* pz = z->parent;
        Node* py = y->parent;
        bool z_is_right = pz && pz->son[1] == z;
        bool y_is_right = py && py->son[1] == y;
        Node* zl = z->son[0];
        Node* zr = z->son[1];
        Node* yl = y->son[0];
        Node* yr = y->son[1];
        Color cz = z->color,  cy = y->color;
        size_t sz = z->siz, sy = y->siz;

        if (pz) pz->son[z_is_right] = y; else root = y;
        if (py && py != z)  py->son[y_is_right] = z; // 如果 y->parent==z，留给下面处理
        else if (!py)      root = z;

        z->parent = (py == z ? y : py);
        y->parent = pz;

        if (py == z) {
            y->son[0] = zl; if (zl) zl->parent = y;
            y->son[1] = z;  z->parent = y;
            z->son[0] = yl; if (yl) yl->parent = z;
            z->son[1] = yr; if (yr) yr->parent = z;
        } else {
            y->son[0] = zl; if (zl) zl->parent = y;
            y->son[1] = zr; if (zr) zr->parent = y;
            z->son[0] = yl; if (yl) yl->parent = z;
            z->son[1] = yr; if (yr) yr->parent = z;
        }

        z->color = cy;
        y->color = cz;

        for (Node* p = z; p != nullptr; p = p->parent) UpdSiz(p);
        for (Node* p = y; p != nullptr; p = p->parent) UpdSiz(p);
    }


public:    
    ESet() : comp(Compare()) {}
    ~ESet() {
        clear();
        for (Node *node : pool)  delete node;
        pool.clear();
    }

    // 深拷贝。最高可接受复杂度 O(nlogn)
    ESet(const ESet& other) : comp(other.comp), root(nullptr), first(nullptr), count(0) {
        copy_subtree(other.root);
    }
    ESet& operator=(const ESet& other) {
        if (this != &other) {
            clear();
            copy_subtree(other.root);
        }
        return *this;
    }

    // 移动。移动之后不应该有新增的空间，other应当被销毁。O(1)
    ESet(ESet&& other) : root(other.root), first(other.first), tail(other.tail), count(other.count), comp(other.comp) {
        other.root = nullptr;  other.first = nullptr;  other.count = 0;
    }
    ESet& operator=(ESet&& other) noexcept {
        if (this != &other) {
            clear();
            comp = other.comp;
            root = other.root;  first = other.first;  count = other.count;  tail = other.tail;
            other.root = nullptr;  other.first = nullptr;  other.count = 0;
        }
        return *this;
    }

    // 返回当前ESet总元素个数， O(1)
    size_t size() const noexcept {return count; }

    // 迭代器操作O(logn)
    class iterator {
        public:
        const Node *it;
        const ESet* container;
    public:
        iterator(Node *oth = nullptr, const ESet* cont = nullptr) : it(oth), container(cont) {}
        iterator(const iterator& other) : it(other.it), container(other.container) {}
        // *it
        const Key& operator*() const {
            if (it == nullptr)  throw;
            return it->key;
        }
        // it++
        iterator operator++(int) {
            if (it == nullptr)  return *this;
            iterator tmp(*this);
            it = it->next;
            return tmp;
        }
        // ++it
        iterator& operator++() {
            if (it == nullptr)  return *this;
            it = it->next;
            return *this;
        }
        // it--
        iterator operator--(int) {
            if (it == container->first)  return *this;
            iterator tmp(*this);
            if (it != nullptr) it = it->prev;
            else {
                it = container->tail;
            }
            return tmp;
        }
        // --it
        iterator& operator--() {
            //std::cout << "in --it" << std::endl;
            if (it == container->first)  return *this;
            //hi();
            if (it != nullptr) it = it->prev;
            else {
                it = container->tail;
            }
            return *this;
        }
        // it1 == it2
        bool operator==(const iterator& other) {return it == other.it; }
        // it1 != it2
        bool operator!=(const iterator& other) {//std::cout << "!=" << std::endl;
            return it != other.it; }
    };

    // O(1)的首/尾迭代器
    iterator begin() const noexcept {return iterator(first, this); }
    iterator end() const noexcept {//std::cout << "end" << std::endl;
        return iterator(nullptr, this); }

    iterator find(const Key& key) const {
        Node *cur = root;
        while (cur != nullptr) {
            int cmp = comp(key, cur->key) ? -1 : (comp(cur->key, key) ? 1 : 0);
            if (cmp == -1) {
                cur = cur->son[0];
            } else if (cmp == 1) {
                cur = cur->son[1];
            } else {
                return iterator(cur, this);
            }
        }
        return end();
    }

    // 返回[l,r]内元素的个数。若l>r，返回0。最高可接受复杂度 O(logn) 
    size_t range(const Key& l, const Key& r) const {
        if (comp(r, l))  return 0;
         //  <l 的数量为ret1， >r 的数量为ret2，之间的元素就是 count - ret1 - ret2
        size_t ret1 = 0, ret2 = 0;
        Node *cur = root;
        while (cur != nullptr) {
            int cmp = comp(l, cur->key) ? -1 : (comp(cur->key, l) ? 1 : 0);
            if (cmp == -1) {
                cur = cur->son[0];
            } else if (cmp == 1) {
                ret1 += 1;
                if (cur->son[0]) {ret1 += cur->son[0]->siz; }
                cur = cur->son[1];
            } else {
                if (cur->son[0]) {ret1 += cur->son[0]->siz; } 
                break;
            }
        }
        cur = root;
        while (cur != nullptr) {
            int cmp = comp(r, cur->key) ? -1 : (comp(cur->key, r) ? 1 : 0);
            if (cmp == -1) {
                ret2 += 1;
                if (cur->son[1]) {ret2 += cur->son[1]->siz; }
                cur = cur->son[0];
            } else if (cmp == 1) {
                cur = cur->son[1];
            } else {
                if (cur->son[1]) {ret2 += cur->son[1]->siz; }
                break;
            }
        }
        return count - ret1 - ret2;
    }

    // 返回最小的 >= key 的元素的迭代器/end()， O(logn) 
    iterator lower_bound(const Key& key) const {
        Node *cur = root, *ans = nullptr;
        while (cur != nullptr) {
            if (!comp(cur->key, key)) {
                ans = cur;
                cur = cur->son[0];
            } else {
                cur = cur->son[1];
            }
        }
        return iterator(ans, this);
    }

    // 返回最小的 > key 的元素的迭代器/end()， O(logn) 
    iterator upper_bound(const Key& key) const {
        Node *cur = root, *ans = nullptr;
        while (cur != nullptr) {
            if (comp(key, cur->key)) {
                ans = cur;
                cur = cur->son[0];
            } else {
                cur = cur->son[1];
            }
        }
        return iterator(ans, this);
    }

    template< class... Args >
    std::pair<iterator, bool> emplace( Args&&... args ) {
        Key key(std::forward<Args>(args)...);
        if (!count) {
            //root = new Node(key);
            root = NEW_NODE(key);
            //all_nodes.insert(root);
            root->color = BLACK;  root->siz = 1;
            first = root;
            tail = root;
            ++count;
            return {begin(), 1};
        }
        // 获取插入的位置(在UpdSiz之前pos是插入节点的父节点)
        Node *pos = root;
        while (true) {
            int cmp = comp(key, pos->key) ? -1 : (comp(pos->key, key) ? 1 : 0);
            if (cmp == -1) {
                if (pos->son[0] == nullptr) break;
                else  pos = pos->son[0];
            } else if (cmp == 1) {
                if (pos->son[1] == nullptr) break;
                else  pos = pos->son[1];
            } else {
                return {iterator(pos, this), 0};
            }
        }
        ++count;
        // *new_node = new Node(key);
        Node *new_node = NEW_NODE(key);
        //all_nodes.insert(new_node);
        new_node->color = RED;
        new_node->siz = 1;
        new_node->parent = pos;
        // 对双红的处理不影响双向链表（中序遍历），可以在前面就处理好prev和next
        if (comp(key, pos->key)) {
            pos->son[0] = new_node;
            Node *pre = pos->prev;
            new_node->next = pos;
            new_node->prev = pre;
            pos->prev = new_node;
            if (pre != nullptr) {pre->next = new_node; }
            else {first = new_node; }
        } else {
            pos->son[1] = new_node;
            Node *nxt = pos->next;
            new_node->prev = pos;
            new_node->next = nxt;
            pos->next = new_node;
            if (nxt != nullptr) {nxt->prev = new_node; }
            else {tail = new_node; }
        }
        while (pos != nullptr) {
            UpdSiz(pos);
            pos = pos->parent;
        }
        
        if (new_node->parent->color == BLACK)  return {iterator(new_node, this), 1};      

        // 处理双红的情况
        // 获取叔叔节点，pos->parent必然存在，因为pos是红的，意味着pos不是根
        Node *cur = new_node;
        while (cur != root && cur->parent->color == RED) {
            Node *father = cur->parent;
            Node *grand = cur->parent->parent;
            Node *uncle = grand->son[father != grand->son[1]];
            if (uncle == nullptr || uncle->color == BLACK) {
                if (father == grand->son[0]) {  // L
                    if (cur == father->son[0]) {  // LL
                        Rotate(grand, 1);
                        father->color = BLACK;  grand->color = RED;
                    } else {  // LR
                        Rotate(father, 0);
                        Rotate(grand, 1);
                        cur->color = BLACK;  grand->color = RED;
                    }
                } else { // R
                    if (cur == father->son[1]) {  // RR
                        Rotate(grand, 0);
                        father->color = BLACK;  grand->color = RED;
                    } else { // RL
                        Rotate(father, 1);
                        Rotate(grand, 0);
                        cur->color = BLACK;  grand->color = RED;
                    }
                }
            } else {
                // 变色
                father->color = uncle->color = BLACK;
                grand->color = RED;
                // 假装插入爷爷，递归处理
                cur = grand;
                if (cur == root)  cur->color = BLACK;
            }
        }
        return {iterator(new_node, this), 1};
    }

    // 有key返回1，无则返回0
    size_t erase(const Key& key) {
        // 找到要删除的节点
        Node *z = root;
        while (z != nullptr) {
            int cmp = comp(key, z->key) ? -1 : (comp(z->key, key) ? 1 : 0);
            if (cmp == -1) {
                z = z->son[0];
            } else if (cmp == 1) {
                z = z->son[1];
            } else {
                break;
            }
        }

        if (z == nullptr)  return 0; 

        // y是实际上要删掉的键
        // 找到替代y的键x（可能是nullptr）
        Node *y = z;
        Color y_original_color = y->color;
        Node *x = nullptr;
        

        if (z->son[0] != nullptr && z->son[1] != nullptr) {
            // z有两个孩子，找后继或者前驱替代，递归处理
            y = z->next;
            // 完全交换y z内存块在结构中的位置
            y_original_color = y->color;
            swap_nodes(z, y);
            y = z;
            x = y->son[1];
        }
        // 如果y只有一个孩子，根据红黑树结构特点，只能是黑y，红x
        if (y->son[0] != nullptr) x = y->son[0];
        else if (y->son[1] != nullptr) x = y->son[1];
        // 如果y是叶子节点，x就还是nullptr不用变

        Node *parent_of_y = y->parent;

        // 把y从树中去掉
        if (x != nullptr) {
            x->parent = y->parent;
        }
        if (y->parent == nullptr) { // y是根
            root = x;
        } else {
            y->parent->son[y == y->parent->son[1]] = x;
        }
        //if (root == y) {std::cout << "y is root" << std::endl;}

        // 处理双黑
        if (y_original_color == BLACK) {
            //std::cout << "extra black" << std::endl;
            Node *current_fixup_node = x; // 当前双黑节点
            Node *current_fixup_parent = parent_of_y; // 双黑节点父节点（逻辑上）

            while (current_fixup_node != root && (current_fixup_node == nullptr || current_fixup_node->color == BLACK)) {
                if (root != nullptr) root->color = BLACK;
                //std::cout << "hey" << std::endl;
                 if (!current_fixup_parent) break;
                // 获取兄弟节点w，w一定存在，否则上一行就break掉了
                Node *w = current_fixup_parent->son[current_fixup_node == current_fixup_parent->son[0]];
                // 兄弟节点是红的
                if (w && w->color == RED) {
                    w->color = BLACK;
                    current_fixup_parent->color = RED;
                    if (current_fixup_parent->son[0] == current_fixup_node)  Rotate(current_fixup_parent, 0);
                    else  Rotate(current_fixup_parent, 1);
                    // 获取新兄弟
                    w = (current_fixup_parent->son[0] == current_fixup_node) ? current_fixup_parent->son[1] : current_fixup_parent->son[0];
                }

                // 兄弟节点是黑的
                Node *wl = nullptr, *wr = nullptr;
                if (w != nullptr) {
                    wl = w->son[0];  wr = w->son[1];
                }
                bool wl_is_black = (!wl || wl->color == BLACK);
                bool wr_is_black = (!wr || wr->color == BLACK);
                //std::cout << "flag" << std::endl;

                if (wl_is_black && wr_is_black) {
                    if (w)  w->color = RED;
                    current_fixup_node = current_fixup_parent; // 双黑上移到父节点
                    current_fixup_parent = current_fixup_parent->parent;
                } else {
                   // 兄弟至少有一个红孩子
                   if (current_fixup_parent->son[0] == current_fixup_node) { // RX
                        if (wr_is_black) { // RL
                            if (wl) wl->color = BLACK;
                            w->color = RED;
                            Rotate(w, 1);
                            w = current_fixup_parent->son[1];
                            //std::cout << "here" << std::endl;
                            wr = w->son[1];
                        }
                        // RR
                        w->color = current_fixup_parent->color;
                        current_fixup_parent->color = BLACK;
                        if (wr) wr->color = BLACK;
                        Rotate(current_fixup_parent, 0);

                        current_fixup_node = root;
                        //std::cout << "rr" << std::endl;
                    } else { // LX
                        if (wl_is_black) { // LR
                            if (wr) wr->color = BLACK;
                            w->color = RED;
                            Rotate(w, 0);
                            w = current_fixup_parent->son[0];
                            wl = w->son[0];
                        }
                        /// LL
                        w->color = current_fixup_parent->color;
                        current_fixup_parent->color = BLACK; 
                        if (wl) wl->color = BLACK;
                        Rotate(current_fixup_parent, 1); 
                        current_fixup_node = root;
                    }

                    break;
                }
            }
            // 当前节点要么是根要么是红的
             if (current_fixup_node != nullptr) current_fixup_node->color = BLACK;
        }

        // 处理完了双黑再统一处理siz和删除
        // 从y原来的父节点开始更新siz
        Node *cur = parent_of_y;
        if (cur == nullptr && root != nullptr) {UpdSiz(root); }
        while (cur != nullptr) {
            UpdSiz(cur);
            cur = cur->parent;
        }
        Del_in_list(y);
        --count;
        //delete y;
        RECYCLE(y);
        return 1;
    }
};