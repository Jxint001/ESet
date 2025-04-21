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
class index_out_of_bound : public exception {};
class runtime_error : public exception {};
class invalid_iterator : public exception {};
class container_is_empty : public exception {};

template <class Key, class Compare = std::less<Key>>
class ESet {
    struct Node{
        Node *left, *right, *parent;
        enum color {BLACK, RED};
        Key key;
    };
    Node* root = nullptr;
    size_t count = 0;

public:    
    ESet();
    ~ESet();

    // 返回当前ESet总元素个数， O(1)
    size_t size() const noexcept {return count; }

    // 迭代器操作要O(1)
    class iterator;

    template< class... Args >
    std::pair<iterator, bool> emplace( Args&&... args );

    size_t erase( const Key& key );  // 有key返回1，无则返回0

    iterator find( const Key& key ) const;

    // 复制
    ESet( const ESet& other );
    ESet& operator=( const ESet& other );

    // 移动。移动之后不应该有新增的空间，other应当被销毁。最高可接受复杂度 O(nlogn) 
    ESet( ESet&& other );
    ESet& operator=( ESet&& other ) noexcept;

    // 返回[l,r]内元素的个数。若l>r，返回0。最高可接受复杂度 O(logn) 
    size_t range( const Key& l, const Key& r ) const;

    // 返回最小的 >= key 的元素的迭代器/end()， O(logn) 
    iterator lower_bound( const Key& key ) const;

    // 返回最小的 > key 的元素的迭代器/end()， O(logn) 
    iterator upper_bound( const Key& key ) const;

    // O(1)的首/尾迭代器
    iterator begin() const noexcept;
    iterator end() const noexcept;
};