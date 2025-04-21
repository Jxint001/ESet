#include <cstddef>
#include <functional>
#include <string>
class exception {
protected:
    const std::string variant = "";
    std::string detail = "";
public:
    exception() {}
    exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
    virtual std::string what() {
        return variant + " " + detail;
    }
};
class invalid_iterator : public exception {};
enum Color {BLACK, RED};
template <class Key, class Compare = std::less<Key>>
class ESet {
    struct Node{
        Node *son[2], *parent, *prev, *next;
        
        Color color;
        Key key;
        size_t siz;
        
        Node(const Key& k) : key(k) {}
    };
    Node* root = nullptr;
    Node* first = nullptr;
    size_t count = 0;

    void UpdSiz(Node* u) {
        Node *cur = u;
        while (cur != nullptr) {
            cur->siz = 1;
            if (cur->son[0]) {cur->siz += cur->son[0]->siz; }
            if (cur->son[1]) {cur->siz += cur->son[1]->siz; }
            cur = cur->parent;
        }
    }

    void clear() {
        Node *cur = first, *nxt;
        while (cur != nullptr) {
            nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        root = nullptr;  first = nullptr;
        count = 0;
    }

    // 以u为旋转中心的旋转，flg为0是左旋，否则右旋
    // 注意旋转操作不影响prev和next，不影响color，影响son[2],parent和siz
    void Rotate(Node* u, bool flg) {
        
    }

public:    
    ESet();
    ~ESet() {clear(); }

    // 深拷贝。最高可接受复杂度 O(nlogn)
    ESet(const ESet& other) {
        for (auto it = other.begin(); it != other.end(); ++it) {
            emplace(*it);
        }
    }
    ESet& operator=(const ESet& other) {
        if (this != &other) {
            clear();
            for (auto it = other.begin(); it != other.end(); ++it) {
                emplace(*it);
            }
        }
        return *this;
    }

    // 移动。移动之后不应该有新增的空间，other应当被销毁。O(1)
    ESet(ESet&& other) : root(other.root), first(other.first), count(other.count) {
        other.root = nullptr;  other.first = nullptr;  other.count = 0;
    }
    ESet& operator=(ESet&& other) noexcept {
        if (this != &other) {
            clear();
            root = other.root;  first = other.first;  count = other.count;
            other.root = nullptr;  other.first = nullptr;  other.count = 0;
        }
        return *this;
    }

    // 返回当前ESet总元素个数， O(1)
    size_t size() const noexcept {return count; }

    // 迭代器操作O(1)
    class iterator {
        const Node* it;
        const ESet* container;
    public:
        iterator(Node* oth, const ESet* cont) : it(oth), container(cont) {}
        // *it
        const Key& operator*() const {
            if (it == nullptr)  throw invalid_iterator();
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
            it = it->prev;
            return tmp;
        }
        // --it
        iterator& operator--() {
            if (it == container->first)  return *this;
            it = it->prev;
            return *this;
        }
        // it1 == it2
        bool operator==(const iterator& other) {return it == other.it; }
        // it1 != it2
        bool operator!=(const iterator& other) {return it != other.it; }
    };

    // O(1)的首/尾迭代器
    iterator begin() const noexcept {return iterator(first, this); }
    iterator end() const noexcept {return iterator(nullptr, this); }

    iterator find(const Key& key) const {
        Node *cur = root;
        while (cur != nullptr) {
            if (key < cur->key) {
                cur = cur->son[0];
            } else if (cur->key < key) {
                cur = cur->son[1];
            } else {
                return iterator(cur, this);
            }
        }
        return end();
    }

    // 返回[l,r]内元素的个数。若l>r，返回0。最高可接受复杂度 O(logn) 
    size_t range(const Key& l, const Key& r) const {
        if (l > r)  return 0;
         //  <l 的数量为ret1， >r 的数量为ret2，之间的元素就是 count - ret1 - ret2
        size_t ret1 = 0, ret2 = 0;
        Node* cur = root;
        while (cur != nullptr) {
            if (l < cur->key) {
                cur = cur->son[0];
            } else if (cur->key < l) {
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
            if (r < cur->key) {
                ret2 += 1;
                if (cur->son[1]) {ret2 += cur->son[1]->siz; }
                cur = cur->son[0];
            } else if (cur->key < r) {
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
            if (key < cur->key || key == cur->key) {
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
            if (key < cur->key) {
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
            root = new Node(key);
            root->color = BLACK;  root->siz = 1;
            first = root;
            ++count;
            return {begin(), 1};
        }
        // 获取插入的位置(pos是插入节点的父节点)
        Node* pos = root;
        while (true) {
            if (key < pos->key) {
                if (pos->son[0] == nullptr) break;
                else  pos = pos->son[0];
            } else if (pos->key < key) {
                if (pos->son[1] == nullptr) break;
                else  pos = pos->son[1];
            } else {
                return {iterator(pos), 0};
            }
        }
        ++count;
        Node *new_node = new Node(key);  new_node->color = RED;  new_node->siz = 1;
        // 对双红的处理不影响双向链表（中序遍历），可以在前面就处理好prev和next
        if (key < pos->key) {
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
        }
        UpdSiz(pos);
        if (pos->color == BLACK)  return {iterator(new_node), 1};      

        // 处理双红的情况
        // 获取叔叔节点，pos->parent必然存在，因为pos是红的，意味着pos不是根
        Node* cur = new_node;
        while (cur != root && cur->parent->color == RED) {
            Node* father = cur->parent;
            Node* grand = cur->parent->parent;
            Node *uncle = grand->son[father != grand->son[1]];
            if (uncle == nullptr || uncle->color = BLACK) {
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
            }
            if (uncle->color == RED) {
                // 变色
                father->color = uncle->color = BLACK;
                grand->color = RED;
                // 假装插入爷爷，递归处理
                cur = grand;
                if (cur == root)  cur->color = BLACK;
            }
        }
        return {iterator(new_node), 1};
    }

    size_t erase(const Key& key);  // 有key返回1，无则返回0
};