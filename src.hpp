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

template <class Key, class Compare = std::less<Key>>
class ESet {
    struct Node{
        Node *son[2], *parent, *prev, *next;
        enum color {BLACK, RED};
        Key key;
    };
    Node* root = nullptr;
    Node* first = nullptr;
    size_t count = 0;

    Node* subtree_min(const Node* u) {}

    Node* subtree_max(const Node* u) {}

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

public:    
    ESet();
    ~ESet() {clear(); }

    // 深拷贝
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

    // 移动。移动之后不应该有新增的空间，other应当被销毁。最高可接受复杂度 O(nlogn) 
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

    // 迭代器操作要O(1)
    class iterator {
        const Node* it;
        const ESet* container;
    public:
        iterator(Node* oth, const ESet* cont) : it(oth), container(cont) {}
        //*it
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

    iterator find(const Key& key) const;

    // 返回[l,r]内元素的个数。若l>r，返回0。最高可接受复杂度 O(logn) 
    size_t range(const Key& l, const Key& r) const;

     // 返回最小的 >= key 的元素的迭代器/end()， O(logn) 
    iterator lower_bound(const Key& key) const;

    // 返回最小的 > key 的元素的迭代器/end()， O(logn) 
    iterator upper_bound(const Key& key) const;

    template< class... Args >
    std::pair<iterator, bool> emplace( Args&&... args );

    size_t erase(const Key& key);  // 有key返回1，无则返回0
};