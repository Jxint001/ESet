#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <memory>
#include <set>
#include <random>
typedef long long ll;
using std::vector, std::cerr;
using NodePtr = std::shared_ptr<struct Node>;

using std::cout, std::endl;
static inline uint64_t normKey(ll x) {
    return (uint64_t) x ^ (1ULL << 63);
}

struct load4 {
    uint8_t key4[4];  NodePtr child4[4];
    load4() { memset(key4, 0, sizeof(key4)); for (int i = 0; i < 4; ++i) child4[i] = nullptr; }
};
struct load16 {
    uint8_t key16[16];  NodePtr child16[16];
    load16() { memset(key16, 0, sizeof(key16)); for (int i = 0; i < 16; ++i) child16[i] = nullptr; }
};
struct load48 {
    int8_t childIndex[256]; NodePtr child48[48];
    load48() { for (int i = 0; i < 256; ++i) childIndex[i] = -1; for (int i = 0; i < 48; ++i) child48[i] = nullptr; }
};
struct load256 {
    NodePtr child256[256];
    load256() { for (int i = 0; i < 256; ++i) child256[i] = nullptr; }
};

struct Node{
    enum NodeType: uint8_t {LEAF, NODE4, NODE16, NODE48, NODE256 } type;
    uint8_t count;  // current # of child nodes
    int size;
    union {
        ll value;
        void *payload;
    } data;

    static inline void Pay(Node *n) {
        if (n->type == Node::LEAF)  return;
        switch (n->type) {
        case Node::NODE4:
            n->data.payload = operator new(sizeof(load4));
            new(n->data.payload) load4();
            break;
        case Node::NODE16:
            n->data.payload = operator new(sizeof(load16));
            new(n->data.payload) load16();
            break;
        case Node::NODE48:
            n->data.payload = operator new(sizeof(load48));
            new(n->data.payload) load48();
            break;
        case Node::NODE256:
            n->data.payload = operator new(sizeof(load256));
            new(n->data.payload) load256();
            break;
        default:
            break;
        }
    }

    Node(NodeType t = LEAF) : type(t), count(0), size(t == LEAF ? 1 : 0) {
        if (type == Node::LEAF) {
            data.value = 0;
        } else {
             Pay(this);
        }
    }

    ~Node() {
        if (type != Node::LEAF && data.payload) {
            switch (type) {
                case Node::NODE4:
                    static_cast<load4*>(data.payload)->~load4();
                    operator delete(data.payload);
                    break;
                case NODE16:
                    static_cast<load16*>(data.payload)->~load16();
                    operator delete(data.payload);
                    break;
                case NODE48:
                    static_cast<load48*>(data.payload)->~load48();
                    operator delete(data.payload);
                    break;
                case NODE256:
                    static_cast<load256*>(data.payload)->~load256();
                    operator delete(data.payload);
                    break;
                default:
                    break;
            }
            data.payload = nullptr;
        }
    }
};

static inline NodePtr cloneNode(const NodePtr &src) {
    // 1) 新建同类型节点，会在构造里自动 Pay(this) 分配 payload 内存
    NodePtr dst = std::make_shared<Node>(src->type);
    // 2) 拷贝元数据
    dst->count = src->count;
    dst->size  = src->size;

    if (src->type == Node::LEAF) {
        // 叶子直接拷 value
        dst->data.value = src->data.value;
    } else {
        
        // 内部节点：逐字段拷贝 key 和 child 指针
        switch (src->type) {
        case Node::NODE4: {
            auto *s = static_cast<load4*>(src->data.payload);
            auto *d = static_cast<load4*>(dst->data.payload);
            for (int i = 0; i < src->count; ++i) {
            d->key4[i]    = s->key4[i];
            d->child4[i]  = s->child4[i];  // shared_ptr 赋值，reference count 正确
            }
            break;
        }
        case Node::NODE16: {
            auto *s = static_cast<load16*>(src->data.payload);
            auto *d = static_cast<load16*>(dst->data.payload);
            for (int i = 0; i < src->count; ++i) {
            d->key16[i]   = s->key16[i];
            d->child16[i] = s->child16[i];
            }
            break;
        }
        case Node::NODE48: {
            auto *s = static_cast<load48*>(src->data.payload);
            auto *d = static_cast<load48*>(dst->data.payload);
            // 拷贝索引映射
            for (int b = 0; b < 256; ++b) {
            d->childIndex[b] = s->childIndex[b];
            }
            // 拷贝实际指针（遍历所有可能有效的子节点）
            for (int b = 0; b < 256; ++b) {
                int idx = s->childIndex[b];
                if (idx != -1) {
                    d->child48[idx] = s->child48[idx];
                }
            }
            break;
        }
        case Node::NODE256: {
            auto *s = static_cast<load256*>(src->data.payload);
            auto *d = static_cast<load256*>(dst->data.payload);
            for (int b = 0; b < 256; ++b) {
                d->child256[b] = s->child256[b];
            }
            break;
        }
        default:
            break;
        }
    }

    return dst;
}

void collectKeys(NodePtr u, vector<ll>& out) {
    if (!u) return;
    if (u->type != Node::LEAF) {
        // 根据 type 递归所有 child
        switch (u->type) {
          case Node::NODE4: {
            auto *p = static_cast<load4*>(u->data.payload);
            for (int i = 0; i < u->count; ++i)
              collectKeys(p->child4[i], out);
            break;
          }
          // 同理写 NODE16 / NODE48 / NODE256
          case Node::NODE16: {
            auto *p = static_cast<load16*>(u->data.payload);
            for (int i = 0; i < u->count; ++i)
              collectKeys(p->child16[i], out);
            break;
          }
          case Node::NODE48: {
            auto *p = static_cast<load48*>(u->data.payload);
            for (int b = 0; b < 256; ++b) {
                int idx = p->childIndex[b];
                if (idx != -1) {
                    collectKeys(p->child48[idx], out);
                }
            }
            break;
          }
          case Node::NODE256: {
            auto *p = static_cast<load256*>(u->data.payload);
            for (int b = 0; b < 256; ++b)
              if (p->child256[b]) collectKeys(p->child256[b], out);
            break;
          }
          default: break;
        }
    } else {
        out.push_back(u->data.value);
    }
}

static inline void check_size(NodePtr n) {
// 叶子节点 size 应为 1
    if (n->type == Node::LEAF) {
        assert(n->size == 1);
        return;
    }

    // 内部节点：累加所有子树的 size
    int sum = 0;
    switch (n->type) {
      case Node::NODE4: {
        auto *p = static_cast<load4*>(n->data.payload);
        for (int i = 0; i < n->count; ++i) {
          sum += p->child4[i]->size;
        }
        break;
      }
      case Node::NODE16: {
        auto *p = static_cast<load16*>(n->data.payload);
        for (int i = 0; i < n->count; ++i) {
          sum += p->child16[i]->size;
        }
        break;
      }
      case Node::NODE48: {
        auto *p = static_cast<load48*>(n->data.payload);
        // load48::child48[0..count-1] 存放了所有子指针
        for (int i = 0; i < n->count; ++i) {
          sum += p->child48[i]->size;
        }
        break;
      }
      case Node::NODE256: {
        auto *p = static_cast<load256*>(n->data.payload);
        // 对于 NODE256，需要遍历所有 256 个可能的子指针
        for (int b = 0; b < 256; ++b) {
          if (p->child256[b]) {
            sum += p->child256[b]->size;
          }
        }
        break;
      }
      default:
        // 不应到这里
        assert(false);
    }

    // 校验：sum(child sizes) == 当前节点记录的 size
    assert(sum == static_cast<int>(n->size));
}

static inline NodePtr makeLeaf(ll value) {
    NodePtr leaf = std::make_shared<Node>(Node::LEAF);
    leaf->data.value = value;
    return leaf;
}

static inline void recalc_size(NodePtr n) {
    if (!n)  return;
    if (n->type == Node::LEAF) {
        n->size = 1;
        return;
    }
    int sum = 0;
    switch (n->type) {
    case Node::NODE4: {
        auto *p = static_cast<load4*>(n->data.payload);
        for (int i = 0; i < n->count; ++i) {
            NodePtr  c = p->child4[i];
            if (c) sum += c->size;
        }
        break;
    }
    case Node::NODE16: {
        auto *p = static_cast<load16*>(n->data.payload);
        for (int i = 0; i < n->count; ++i) {
            NodePtr  c = p->child16[i];
            if (c) sum += c->size;
        }
        break;
    }
    case Node::NODE48: {
        auto *p = static_cast<load48*>(n->data.payload);
        for (int b = 0; b < 256; ++b) {
            int idx = p->childIndex[b];
            if (idx != -1 && p->child48[idx]) {
                sum += p->child48[idx]->size;
            }
        }
        break;
    }
    case Node::NODE256: {
        auto *p = static_cast<load256*>(n->data.payload);
        for (int i = 0; i < 256; ++i) {
            NodePtr  c = p->child256[i];
            if (c) sum += c->size;
        }
        break;
    }
    default: break;
    }
    n->size = sum;
}

// find ch (child) in node
static inline NodePtr findChild(NodePtr node, uint8_t ch) {
    switch (node->type) {
    case Node::NODE4: {
        auto *p = static_cast<load4*>(node->data.payload);
        for (int i = 0; i < node->count; ++i) {
            if (p->key4[i] == ch)  return p->child4[i];
        }
        return nullptr;
    }
    case Node::NODE16: {
        auto *p = static_cast<load16*>(node->data.payload);
        for (int i = 0; i < node->count; ++i) {
            if (p->key16[i] == ch)  return p->child16[i];
        }
        return nullptr;
    }
    case Node::NODE48: {
        auto *p = static_cast<load48*>(node->data.payload);
        int idx = p->childIndex[ch];
        return idx < 0 ? nullptr : p->child48[idx];
    }
    case Node::NODE256: {
        auto *p = static_cast<load256*>(node->data.payload);
        return p->child256[ch];
    }
    default:
        return nullptr;
    }
}

// locate ch's index in node
static inline int findChildIndex(NodePtr node, uint8_t ch) {
    switch (node->type) {
    case Node::NODE4: {
        auto *p = static_cast<load4*>(node->data.payload);
        for (int i = 0; i < node->count; ++i) {
            if (p->key4[i] == ch)  return i;
        }
        break;
    }
    case Node::NODE16: {
        auto *p = static_cast<load16*>(node->data.payload);
        for (int i = 0; i < node->count; ++i) {
            if (p->key16[i] == ch)  return i;
        }
        break;
    }
    case Node::NODE48: {
        auto *p = static_cast<load48*>(node->data.payload);
        int idx = p->childIndex[ch];
        if (idx != -1)  return idx;
        break;
    }
    case Node::NODE256:
        return ch;
    default: break;
    }
    return -1;
}

// assume branch b does not exist previously
// add a new branch (b -> child) to current node (node)
// if count reaches the capacity (no place for insertion), return false
static inline bool insertChild(NodePtr node, uint8_t b, NodePtr child) {
    if (!child)  return false;  // do not //retain while using nullptr for occupation
    //retain(child);
    switch (node->type) {
    case Node::NODE4:
        if (node->count < 4) {
            auto *p = static_cast<load4*>(node->data.payload);
            int pos = node->count++;
            p->key4[pos] = b;
            p->child4[pos] = child;
            // insertion sort to keep key[] sorted
            for (int i = pos; i > 0 && p->key4[i] < p->key4[i-1]; --i) {
                std::swap(p->key4[i], p->key4[i-1]);
                std::swap(p->child4[i], p->child4[i-1]);
            }
            return true;
        }
        break;
    case Node::NODE16:
        if (node->count < 16) {
            auto *p = static_cast<load16*>(node->data.payload);
            int pos = node->count++;
            p->key16[pos] = b;
            p->child16[pos] = child;
            for (int i = pos; i > 0 && p->key16[i] < p->key16[i-1]; i--) {
                std::swap(p->key16[i], p->key16[i-1]);
                std::swap(p->child16[i], p->child16[i-1]);
            }
            return true;
        }
        break;
    case Node::NODE48:
        if (node->count < 48) {
            auto *p = static_cast<load48*>(node->data.payload);
            int pos = node->count++;
            p->childIndex[b] = pos;
            p->child48[pos] = child;
            return true;
        }
        break;
    case Node::NODE256: {
        auto *p = static_cast<load256*>(node->data.payload);
        if (!p->child256[b]) {
            p->child256[b] = child;
            node->count++;
            return true;
        }
        break;
    }
    default: break;
    }
    return false;
}

// expand node
static inline NodePtr expand(NodePtr node) {
    NodePtr cloned = cloneNode(node);
    Node::NodeType t;
    switch (node->type) {
    case Node::NODE4:
        t = Node::NODE16;
        break;
    case Node::NODE16:
        t = Node::NODE48;
        break;
    case Node::NODE48:
        t = Node::NODE256;
        break;
    default: break;
    }
    NodePtr newnode = std::make_shared<Node>(t);
    switch (node->type) {
    case Node::NODE4: {
        auto *p = static_cast<load4*>(cloned->data.payload);
        for (int i = 0; i < node->count; ++i)
            insertChild(newnode, p->key4[i], p->child4[i]);
        break;
    }
    case Node::NODE16: {
        auto *p = static_cast<load16*>(cloned->data.payload);
        for (int i = 0; i < node->count; ++i)
            insertChild(newnode, p->key16[i], p->child16[i]);
        break;
    }
    case Node::NODE48: {
        auto *p = static_cast<load48*>(cloned->data.payload);
        NodePtr newBig = std::make_shared<Node>(Node::NODE256);
        for (int byte = 0; byte < 256; ++byte) {
            int idx = p->childIndex[byte];
            if (idx >= 0) {
                insertChild(newBig, byte, p->child48[idx]);
            }
        }
        newnode = newBig;
        break;
    }
    default:  return node;
    }
    //newnode->size = node->size;
    recalc_size(newnode);
    check_size(newnode);
    return newnode;
}

static NodePtr insertNode(const NodePtr& node, uint64_t key, ll value, int depth, bool& inserted) {
    // 1) 空树 → 新叶
    if (!node) {
        inserted = true;
        return makeLeaf(value);
    }

    // 2) 叶子 → split
    if (node->type == Node::LEAF) {
        ll oldVal = node->data.value;
        if (oldVal == value) {
            inserted = false;
            return node;            // 重复，直接返回原叶
        }
        uint64_t oldKey = normKey(oldVal);
        // 找到第一个分叉层 d2（从 depth 开始）
        int d2 = depth;
        while (d2 < 8 &&
               (((oldKey ^ key) >> ((7 - d2) * 8)) & 0xFF) == 0) {
            ++d2;
        }
        assert(d2 < 8);

        // 在 d2 层做真正的二分支 NODE4
        NodePtr branch = std::make_shared<Node>(Node::NODE4);
        uint8_t ob = (oldKey >> ((7 - d2) * 8)) & 0xFF;
        uint8_t nb = (key    >> ((7 - d2) * 8)) & 0xFF;
        insertChild(branch, ob, makeLeaf(oldVal));
        insertChild(branch, nb, makeLeaf(value));
        recalc_size(branch);

        // 如果 depth < d2，要把这棵分支外层用 prefix 节点“包回去”
        NodePtr subtree = branch;
        for (int d = d2 - 1; d >= depth; --d) {
            NodePtr prefix = std::make_shared<Node>(Node::NODE4);
            uint8_t b = (oldKey >> ((7 - d) * 8)) & 0xFF;
            insertChild(prefix, b, subtree);
            recalc_size(prefix);
            subtree = prefix;
        }
        inserted = true;
        check_size(subtree);
        return subtree;
    }

     // 3) 内部节点 → 路径拷贝
    NodePtr newNode = cloneNode(node);

    // 查找要插入的分支字节 b
    uint8_t b = (key >> ((7 - depth) * 8)) & 0xFF;
    NodePtr child = findChild(node, b);
    if (child) {
        // 3.1) 递归修改子节点
        bool childInserted = false;
        NodePtr updated = insertNode(child, key, value, depth + 1, childInserted);
        if (!childInserted) { inserted = false; return node; }
        // 3.2) 在 newNode 的 payload 里直接替换对应指针
        switch (newNode->type) {
          case Node::NODE4: {
            auto *p = static_cast<load4*>(newNode->data.payload);
            for (int i = 0; i < newNode->count; ++i)
              if (p->key4[i] == b) { p->child4[i] = updated; break; }
            break;
          }
          case Node::NODE16: {
            auto *p = static_cast<load16*>(newNode->data.payload);
            for (int i = 0; i < newNode->count; ++i)
              if (p->key16[i] == b) { p->child16[i] = updated; break; }
            break;
          }
          case Node::NODE48: {
            auto *p = static_cast<load48*>(newNode->data.payload);
            int idx = p->childIndex[b];
            p->child48[idx] = updated;
            break;
          }
          case Node::NODE256: {
            auto *p = static_cast<load256*>(newNode->data.payload);
            p->child256[b] = updated;
            break;
          }
          default: break;
        }
        inserted = true;
    } else {
        // 3.1) 原子插入新分支（并在满时 expand）
        NodePtr leaf = makeLeaf(value);
        if (!insertChild(newNode, b, leaf)) {
            newNode = expand(newNode);
            insertChild(newNode, b, leaf);
        }
        inserted = true;
    }
    // 3.3) 重算 size 并返回
    recalc_size(newNode);
    check_size(newNode);
    return newNode;
}

// erase key in node's subtree, return the new subtree root
static NodePtr eraseNode(NodePtr node, uint64_t key, ll value, int dep, bool& removed) {
    if (!node) {removed = false;  return nullptr; }
    if (node->type == Node::LEAF) {
        if (node->data.value == value) {
            removed = true;
            return nullptr;
        }
        removed = false;
        return node;
    }
    
    NodePtr copy = cloneNode(node);

    uint8_t b = (key >> ((7-dep)*8)) & 0xFF;
    NodePtr  child = findChild(node, b);
    if (!child) {
        removed = false;
        return node;
    }

    NodePtr updated = eraseNode(child, key, value, dep + 1, removed);
    if (!removed) {
        return node;
    }
    if (!updated) {
        // remove branch b
        switch (copy->type) {
        case Node::NODE4: {
            auto *pc = static_cast<load4*>(copy->data.payload);
            for (int i = 0; i < copy->count; ++i) {
                if (pc->key4[i] == b) {
                    // move forward
                    for (int j = i+1; j < copy->count; ++j) {
                        pc->key4[j-1] = pc->key4[j];
                        pc->child4[j-1] = pc->child4[j];
                    }
                    copy->count--;
                    break;
                }
            }
            break;
        }
        case Node::NODE16: {
            auto *pc = static_cast<load16*>(copy->data.payload);
            for (int i = 0; i < copy->count; ++i) {
                if (pc->key16[i] == b) {
                    for (int j = i+1; j < copy->count; ++j) {
                    pc->key16[j-1] = pc->key16[j];
                    pc->child16[j-1] = pc->child16[j];
                    }
                    copy->count--;
                    break;
                }
            }
            break;
        }
        case Node::NODE48: {
            auto *pc = static_cast<load48*>(copy->data.payload);
            int idx = pc->childIndex[b];
            if (idx >= 0) {
                int last = copy->count - 1;
                pc->child48[idx] = pc->child48[last];
                // update childIndex
                for (int c = 0; c < 256; ++c) {
                    if (pc->childIndex[c] == last) {
                        pc->childIndex[c] = idx;
                        //break;
                    }
                }

                pc->child48[last] = nullptr;
                pc->childIndex[b] = -1;
                copy->count--;
            }
            break;
        }
        case Node::NODE256: {
            auto *pc = static_cast<load256*>(copy->data.payload);
            if (pc->child256[b]) {
                pc->child256[b] = nullptr;
                copy->count--;
            }
            break;
        }
        default: break;
        }
    } else {
        // replace child[b] with updated
        switch (copy->type) {
        case Node::NODE4: {
            auto *pc = static_cast<load4*>(copy->data.payload);
            for (int i = 0; i < copy->count; ++i) {
                if (pc->key4[i] == b) {
                    pc->child4[i] = updated;
                    break;
                }
            }
            break;
        }
        case Node::NODE16: {
            auto *pc = static_cast<load16*>(copy->data.payload);
            for (int i = 0; i < copy->count; ++i) {
                if (pc->key16[i] == b) {
                    pc->child16[i] = updated;
                    break;
                }
            }
            break;
        }
        case Node::NODE48: {
            auto *pc = static_cast<load48*>(copy->data.payload);
            int idx = pc->childIndex[b];
            pc->child48[idx] = updated;
            break;
            }
        case Node::NODE256: {
            auto *pc = static_cast<load256*>(copy->data.payload);
            pc->child256[b] = updated;
            break;
        }
        default: break;
        }
    }
    removed = true;
    if (copy->count == 0) {
        return nullptr;
    }
    recalc_size(copy);
    check_size(copy);
    return copy;
}

class ESet{
    // return the # of nodes that <= x
    size_t rank(const ll& x) const {
        uint64_t nk = normKey(x);
        NodePtr  cur = root;
        size_t cnt = 0;

        for (int d = 0; d < 8 && cur ; ++d) {
            uint8_t b = (nk >> ((7-d)*8)) & 0xFF;
            switch (cur->type) {
            case Node::LEAF: {
                if (cur->data.value <= x)  cnt += 1;
                break;
            }
            case Node::NODE4: {
                auto *pu = static_cast<load4*>(cur->data.payload);
                for (int i = 0; i < cur->count && pu->key4[i] < b; ++i)  cnt += pu->child4[i]->size;
                break;
            }
            case Node::NODE16: {
                auto *pu = static_cast<load16*>(cur->data.payload);
                for (int i = 0; i < cur->count && pu->key16[i] < b; ++i)  cnt += pu->child16[i]->size;
                break;
            }
            case Node::NODE48: {
                auto *pu = static_cast<load48*>(cur->data.payload);
                for (int byte = 0; byte < b; ++byte) {
                    int idx = pu->childIndex[byte];
                    if (idx != -1 && pu->child48[idx])  cnt += pu->child48[idx]->size;
                }
                break;
            }
            case Node::NODE256: {
                auto *pu = static_cast<load256*>(cur->data.payload);
                for (int byte = 0; byte < b; ++byte) {
                    NodePtr  c = pu->child256[byte];
                    if (c) {
                        cnt += c->size;
                    }
                }
                break;
            }
            default:  break;
            }
            cur = findChild(cur, b);
        }

        if (cur && cur->type == Node::LEAF && cur->data.value <= x)  cnt += 1;
        return cnt;
    }

   
public:
 NodePtr root;
    struct iterator{
        const ESet* cont;
        NodePtr  path[9];  // level 0 to 8
        int idx[9];
        bool is_end;

        void subtree_min(NodePtr  cur, int d) {
            while (cur && cur->type != Node::LEAF) {
                path[d] = cur;
                int sel = 0;
                switch (cur->type) {
                case Node::NODE4:
                case Node::NODE16:
                    sel = 0;
                    break;
                case Node::NODE48: {
                    auto *pu = static_cast<load48*>(cur->data.payload);
                    for (int b = 0; b < 256; ++b) {
                        if (pu->childIndex[b] != -1) {
                            sel = pu->childIndex[b];
                            break;
                        }
                    }
                    break;
                }
                case Node::NODE256: {
                    auto *pu = static_cast<load256*>(cur->data.payload);
                    for (int b = 0; b < 256; ++b) {
                        if (pu->child256[b]) {
                            sel = b;
                            break;
                        }
                    }
                    break;
                }
                default: break;
                }
                idx[d] = sel;
                switch (cur->type) {
                case Node::NODE4: {
                    auto *pu = static_cast<load4*>(cur->data.payload);
                    cur = pu->child4[sel];
                    break;
                }
                case Node::NODE16: {
                    auto *pu = static_cast<load16*>(cur->data.payload);
                    cur = pu->child16[sel];
                    break;
                }
                case Node::NODE48: {
                    auto *pu = static_cast<load48*>(cur->data.payload);
                    cur = pu->child48[sel];
                    break;
                }
                case Node::NODE256: {
                    auto *pu = static_cast<load256*>(cur->data.payload);
                    cur = pu->child256[sel];
                    break;
                }
                default: cur = nullptr;  break;
                }
                ++d;
            }
            path[d] = cur;  // leaf or nullptr
            for (int i = d + 1; i < 9; ++i) {
                path[i] = nullptr;
                idx[i] = 0;
            }
        }

        void subtree_max(NodePtr  cur, int d) {
            while (cur && cur->type != Node::LEAF) {
                path[d] = cur;
                int sel = -1;
                switch (cur->type) {
                case Node::NODE4:
                case Node::NODE16:
                    sel = cur->count - 1;
                    break;
                case Node::NODE48: {
                    // 在 childIndex 中找到最大 byte
                    auto *pu = static_cast<load48*>(cur->data.payload);
                    for (int b = 255; b >= 0; --b) {
                        int8_t ci = pu->childIndex[b];
                        if (ci != -1) {
                            sel = ci;
                            break;
                        }
                    }
                    break;
                }
                case Node::NODE256: {
                    // 在 child 数组中找到最大 byte
                    auto *pu = static_cast<load256*>(cur->data.payload);
                    for (int b = 255; b >= 0; --b) {
                        if (pu->child256[b]) {
                            sel = b;
                            break;
                        }
                    }
                    break;
                }
                default: break;
                }
                idx[d] = sel;
                // 进入下一层
                switch (cur->type) {
                case Node::NODE4: {
                    auto *pu = static_cast<load4*>(cur->data.payload);
                    cur = pu->child4[sel];
                    break;
                }
                case Node::NODE16: {
                    auto *pu = static_cast<load16*>(cur->data.payload);
                    cur = pu->child16[sel];
                    break;
                }
                case Node::NODE48: {
                    auto *pu = static_cast<load48*>(cur->data.payload);
                    cur = pu->child48[sel];
                    break;
                }
                case Node::NODE256: {
                    auto *pu = static_cast<load256*>(cur->data.payload);
                    cur = pu->child256[sel];
                    break;
                }
                default: cur = nullptr;  break;
                }
                ++d;
            }
            path[d] = cur;  // leaf or nullptr
            for (int i = d + 1; i < 9; ++i) {
                path[i] = nullptr;
                idx[i] = 0;
            }
        }

        iterator(const ESet* x = nullptr): is_end(true), cont(x) {
            for (int i = 0; i < 9; ++i) {
                path[i] = nullptr;
                idx [i] = 0;
            }
        }

        ll operator*() const {
            if (is_end)  throw std::out_of_range("dereference end or invalid iterator");
            // find the deepest non‐null leaf in the path array
            for (int i = 8; i >= 0; --i) {
                if (path[i] && path[i]->type == Node::LEAF)
                return path[i]->data.value;
            }
            throw std::out_of_range("dereference end or invalid iterator");
        }

        // ++it
        iterator& operator++() {
            if (is_end) return *this;
            // find brother upward
            for (int d = 7; d >= 0; --d) {
                NodePtr  nd = path[d];
                if (!nd || nd->type == Node::LEAF) continue;
                int next_idx = -1;
                // find next_idx
                switch (nd->type) {
                case Node::NODE4:
                case Node::NODE16:
                    next_idx = idx[d] >= nd->count - 1 ? -1 : idx[d] + 1;
                    break;
                case Node::NODE48:{
                    int cur_slot = idx[d];
                    int cur_byte = -1;
                    auto *pd = static_cast<load48*>(nd->data.payload);

                    for (int b = 0; b < 256; ++b) {
                        if (pd->childIndex[b] == cur_slot) {
                            cur_byte = b;  break;
                        }
                    }
                    int next_byte = -1;
                    for (int b = cur_byte + 1; b < 256; ++b) {
                        if (pd->childIndex[b] != -1) {
                            next_byte = b;
                            break;
                        }
                    }
                    if (next_byte != -1) {
                        int next_slot = pd->childIndex[next_byte];
                        next_idx = next_slot;
                    }
                }
                    break;
                case Node::NODE256: {
                    auto *pd = static_cast<load256*>(nd->data.payload);
                    for (int i = idx[d] + 1; i < 256; ++i) {
                        NodePtr c = pd->child256[i];
                        if (c) {next_idx = i;  break; }
                    }
                    break;
                }
                default: break;
                }

                if (next_idx < 0) continue;
                idx[d] = next_idx;
                NodePtr child(nullptr);
                switch (nd->type) {
                case Node::NODE4: {
                    auto *pd = static_cast<load4*>(nd->data.payload);
                    child = pd->child4[next_idx];
                    break;
                }
                case Node::NODE16: {
                    auto *pd = static_cast<load16*>(nd->data.payload);
                    child = pd->child16[next_idx];
                    break;
                }
                case Node::NODE48: {
                    auto *pd = static_cast<load48*>(nd->data.payload);
                    child = pd->child48[next_idx];
                    break;
                }
                case Node::NODE256: {
                    auto *pd = static_cast<load256*>(nd->data.payload);
                    child = pd->child256[next_idx];
                    break;
                }
                default: child = nullptr; break;
                }
                
                subtree_min(child, d+1);
                is_end = false;
                return *this;
            }
            is_end = true;
            return *this;
        }

        // --it
        iterator& operator--() {
            if (!cont->root || *this == cont->begin())  return *this;
            if (is_end) {
                subtree_max(cont->root, 0);
                is_end = cont->root == nullptr;
                return *this;
            }
            for (int d = 7; d >= 0; --d) {
                NodePtr nd = path[d];
                if (!nd || nd->type == Node::LEAF) continue;
                int prev_idx = -1;
                // find prev_idx
                switch (nd->type) {
                case Node::NODE4:
                case Node::NODE16:
                    prev_idx = idx[d] <= 0 ? -1 : idx[d] - 1;
                    break;
                case Node::NODE48:{
                    int cur_slot = idx[d];
                    int cur_byte = -1;
                    auto *pd = static_cast<load48*>(nd->data.payload);

                    for (int b = 0; b < 256; ++b) {
                        if (pd->childIndex[b] == cur_slot) {
                            cur_byte = b;
                            break;
                        }
                    }
                    int prev_byte = -1;
                    for (int b = cur_byte - 1; b >= 0; --b) {
                        if (pd->childIndex[b] != -1) {
                            prev_byte = b;
                            break;
                        }
                    }
                    if (prev_byte != -1) {
                        int prev_slot = pd->childIndex[prev_byte];
                        prev_idx = prev_slot;
                    }
                }
                    break;
                case Node::NODE256: {
                    auto *pd = static_cast<load256*>(nd->data.payload);
                    for (int i = idx[d] - 1;  i >= 0; --i) {
                        NodePtr c = pd->child256[i];
                        if (c) {prev_idx = i;  break; }
                    }
                    break;
                }
                default: break;
                }

                if (prev_idx < 0) continue;
                idx[d] = prev_idx;
                NodePtr  child = nullptr;
                switch (nd->type) {
                case Node::NODE4: {
                    auto *pd = static_cast<load4*>(nd->data.payload);
                    child = pd->child4[prev_idx];
                    break;
                }
                case Node::NODE16: {
                    auto *pd = static_cast<load16*>(nd->data.payload);
                    child = pd->child16[prev_idx];
                    break;
                }
                case Node::NODE48: {
                    auto *pd = static_cast<load48*>(nd->data.payload);
                    child = pd->child48[prev_idx];
                    break;
                }
                case Node::NODE256: {
                    auto *pd = static_cast<load256*>(nd->data.payload);
                    child = pd->child256[prev_idx];
                    break;
                }
                default: child = nullptr; break;
                }
                
                subtree_max(child, d+1);
                is_end = false;
                return *this;
            }
            is_end = true;
            return *this;
        }

        friend bool operator==(iterator const &a, iterator const &b) {
            if (a.is_end && b.is_end) return true;
            if (a.is_end != b.is_end) return false;
            return *a == *b;
        }
        friend bool operator!=(iterator const &a, iterator const &b) {
            return !(a == b);
        }
    };
    ESet() : root(nullptr) {}
    ~ESet() {}

    ESet(const ESet& other): root(other.root) {}//retain(root); }
    ESet& operator=(const ESet& other) {
        if (&other != this) {
            root = other.root;
        }
        return *this;
    }

    iterator begin() const noexcept {
        iterator it(this);
        if (!root) {it.is_end = true;  return it; }
        it.is_end = false;
        it.subtree_min(root, 0);
        return it;
    }
    // end(): is_end = true, no other data needed
    iterator end() const {
        return iterator(this);
    }

    size_t range(const ll& l, const ll& r) const {
        if (l > r || !root) return 0;

        if (l == std::numeric_limits<ll>::min())
            return rank(r);
        else
            return rank(r) - rank(l - 1);
    }
    
    iterator find(const ll& key) const {
        iterator it(this);
        for (int i = 0; i < 9; ++i) {
            it.path[i] = root;
            it.idx[i]  = 0;
        }
        uint64_t nk = normKey(key);
        NodePtr  cur = root;
        int d;
        for (d = 0; d < 8 && cur && cur->type != Node::LEAF; ++d) {
            it.path[d] = cur;
            uint8_t b = (nk >> ((7-d)*8)) & 0xFF;
            it.idx[d] = findChildIndex(cur, b);
            if (it.idx[d] == -1) {
                return end();
            } else {
                switch (cur->type) {
                case Node::NODE4: {
                    auto *pc = static_cast<load4*>(cur->data.payload);
                    cur = pc->child4[it.idx[d]];
                    break;
                }
                case Node::NODE16: {
                    auto *pc = static_cast<load16*>(cur->data.payload);
                    cur = pc->child16[it.idx[d]];
                    break;
                }
                case Node::NODE48: {
                    auto *pc = static_cast<load48*>(cur->data.payload);
                    cur = pc->child48[it.idx[d]];
                    break;
                }
                case Node::NODE256: {
                    auto *pc = static_cast<load256*>(cur->data.payload);
                    cur = pc->child256[it.idx[d]];
                }
                default: break;
                }
            }
        }
        for (; d < 8; ++d) {
            it.path[d] = cur;
            it.idx[d]  = 0;
        }
        it.path[8] = cur;
        it.is_end = !(cur && cur->type == Node::LEAF && cur->data.value == key);
        return it;
    }

    std::pair<iterator, bool> emplace(const ll& key) {
        bool inserted = false;
        NodePtr newroot = insertNode(root, normKey(key), key, 0, inserted);
        if (inserted) {
            root = newroot;
        }
        iterator it = find(key);
        return {it, inserted};
    }
  
    size_t erase(const ll& v) {
        bool removed = false;
        NodePtr newroot = eraseNode(root, normKey(v), v, 0, removed);
        if (removed) {
            if (root)  root = newroot;
        }
        return removed;
    }
};

std::vector<ESet> s;

inline ll MAX(const ll& a, const ll& b) {return a < b ? b : a; }
// 用来数第几次执行 range
static long long RANGE_OP_COUNT = 0;
// 你发现第 72880 次 range 结果是错的
static const long long TARGET_RANGE_OP = 12127;

int dfs_check_size(NodePtr u) {
    if (!u) return 0;
    if (u->type == Node::LEAF) {
        assert(u->size == 1);
        return 1;
    }
    int sum = 0;
    switch (u->type) {
        case Node::NODE4: {
            auto *p = static_cast<load4*>(u->data.payload);
            for (int i = 0; i < u->count; ++i)
                sum += dfs_check_size(p->child4[i]);
            break;
        }
        case Node::NODE16: {
            auto *p = static_cast<load16*>(u->data.payload);
            for (int i = 0; i < u->count; ++i)
                sum += dfs_check_size(p->child16[i]);
            break;
        }
        case Node::NODE48: {
            auto *p = static_cast<load48*>(u->data.payload);
            for (int b = 0; b < 256; ++b) {
                int idx = p->childIndex[b];
                if (idx != -1)
                    sum += dfs_check_size(p->child48[idx]);
            }
            break;
        }
        case Node::NODE256: {
            auto *p = static_cast<load256*>(u->data.payload);
            for (int b = 0; b < 256; ++b) {
                if (p->child256[b])
                    sum += dfs_check_size(p->child256[b]);
            }
            break;
        }
        default: break;
    }
    assert(u->size == sum);
    return sum;
}

// sanity_check：和 std::set 做对比，并且检查 size 不变量
void sanity_check(NodePtr root, const std::set<ll>& ref) {
    // 1) collectKeys
    std::vector<ll> got;
    collectKeys(root, got);

    // 2) ref 的 in-order
    std::vector<ll> exp(ref.begin(), ref.end());

    // 3) 对比顺序、数量
    if (got != exp) {
        std::cerr << "[SANITY] mismatch!\n GOT: ";
        for (auto &x : got) std::cerr << x << ' ';
        std::cerr << "\nEXP: ";
        for (auto &x : exp) std::cerr << x << ' ';
        std::cerr << std::endl;
        assert(false);
    }

    // 4) 检查 size 字段
    dfs_check_size(root);
}

void mytest() {
    std::vector<ESet> s(2);
    std::vector<std::set<ll>> std(2);
    for (ll i = 0; i < 48; ++i) {
        s[0].emplace(i);
        std[0].emplace(i);
    }
    std::vector<ll> got;
    collectKeys(s[0].root, got);
    std::vector<ll> exp(std[0].begin(), std[0].end());
     if (got != exp) {
        std::cerr << "[SANITY] mismatch!\n GOT: ";
        for (auto &x : got) std::cerr << x << ' ';
        std::cerr << "\nEXP: ";
        for (auto &x : exp) std::cerr << x << ' ';
        std::cerr << std::endl;
        assert(false);
    }

    int k = 10;
    s[1] = s[0];  s[1].erase(k);
    got.clear();
    collectKeys(s[1].root, got);
    assert(got.size() == 47);
}

// int main() {
//     vector<std::set<ll>> ref;
    
//     s.reserve(1024);
//     freopen("task3s2.in", "r", stdin);
//     freopen("shared_part.out", "w", stdout);

//     ESet::iterator it;
//     //flag();
//     int op, lst=0, it_a=-1, valid = 0;
//     int t = 0;
//     while (scanf("%d", &op) != EOF) {
//         //cout << t++ << endl;
//         long long a, b, c;
//         fflush(stdout);
//         switch (op) {
//             case 0: {
//                 scanf("%lld%lld", &a, &b);
//                 if ((size_t)a >= s.size())  s.resize(a + 1);
//                 auto p=s[a].emplace(b);
//                 if(p.second) {
//                 	it_a = a;
//                     it = p.first;
//                     valid = 1;
//                 }
//                 break;
//             }
//             case 1:
//                 scanf("%lld%lld", &a, &b);
//                 if ((size_t)a >= s.size())  s.resize(a + 1);
//                 if (valid && it_a==a && *it == b)valid = 0;
//                 s[a].erase(b);
//                 break;
//             case 2:
//                 scanf("%lld", &a);
//                 ++lst;
//                 if ((size_t)a >= s.size() || (size_t)lst >= s.size())  s.resize(MAX(lst + 1, a + 1));
//                 s[lst] = s[a];
//                 break;
//             case 3: {
//                 scanf("%lld%lld", &a, &b);
//                 if ((size_t)a >= s.size())  s.resize(a + 1);
//                 auto it2 = s[a].find(b);
//                 if (it2 != s[a].end()) {
//                     printf("true\n");
//                     it_a = a;
//                     it = it2;
//                     valid = 1;
//                 } else
//                     printf("false\n");
//                 break;
//             }
//             case 4: {
//                 RANGE_OP_COUNT++;
//                 scanf("%lld%lld%lld", &a, &b, &c);
//                 if ((size_t)a >= s.size())  s.resize(a + 1);
//                 printf("%zu\n", s[a].range(b, c));
//                 size_t ans = s[a].range(a,b);
//                 sanity_check(s[a].root, ref[a]);
//                 printf("%zu\n", ans);

//                 if (RANGE_OP_COUNT == TARGET_RANGE_OP) exit(0);
//                 break;
//             }
//             case 5:
//                 // get prev
//                 if (valid && it != s[it_a].begin()) {
//                     --it;
//                     printf("%lld\n", *it);
//                 } else {
//                     valid = 0;
//                     printf("-1\n");
//                 }
//                 break;
//             case 6:
//                 // get next
//                 if (valid) {
//                     ++it;
//                     if (it != s[it_a].end()) {
//                         printf("%lld\n", *it);
//                     } else {
//                         valid = 0;
//                         printf("-1\n");
//                     }
//                 } else {
//                     printf("-1\n");
//                 }
//                 break;
//         }
//     }
//     return 0;
// }
void testVersionIsolation() {
    NodePtr v1 = ESet().root;
    bool inserted;
    NodePtr v2 = insertNode(v1, 0x1234, 42, 0, inserted);
    
    // 验证v1结构完整
    vector<ll> keys1;
    collectKeys(v1, keys1);
    assert(!keys1.empty());
    
    // 验证v2包含新增key
    vector<ll> keys2;
    collectKeys(v2, keys2);
    assert(std::find(keys2.begin(), keys2.end(), 42) != keys2.end());
    
    // 验证版本隔离
    assert(v1 != v2);
}

int main() {
    //testVersionIsolation();
    mytest();
    freopen("error.log", "w", stderr);
    freopen("task3s2.in", "r", stdin);
    freopen("shared_part.out", "w", stdout);
    vector<ESet> es;
    vector<std::set<ll>> ref;
    es.reserve(1024);
    ref.reserve(1024);

    int op;
    ll a, b, c;
    ESet::iterator it;    // 用于 case 5/6 的前驱后继
    int last_version = 0, it_version = -1;
    bool valid_it = false;

    while (scanf("%d", &op) == 1) {
        fflush(stdout);
        switch (op) {
            case 0: { // emplace
                scanf("%lld%lld", &a, &b);
                if ((size_t)a >= es.size()) {
                    es.resize(a+1);
                    ref.resize(a+1);
                }
                auto [it_new, ins_es] = es[a].emplace(b);
                bool ins_ref = ref[a].insert(b).second;
                // 校验不变量
                #ifdef DEBUG
                sanity_check(es[a].root, ref[a]);
                #endif
                // {
                //     std::vector<ll> got;
                //     collectKeys(es[0].root, got);
                //     std::cerr << ">>> ESet[" << a << "] has " << got.size() << " keys, std::set[0] has "
                //             << ref[0].size() << " keys\n";
                // // 只打印前后几条就够看，别都打印了
                //     for (int i = std::max(0, (int)got.size()-5); i < (int)got.size(); ++i)
                //         std::cerr << got[i] << " ";
                //     std::cerr << "\n";
                //     auto it = ref[0].end(); if (it!=ref[0].begin()) { --it;
                //     std::cerr << " std::set last: " << *it << "\n";
                //     }
                // }
                if (ins_es != ins_ref) {
                    std::cerr
                        << "!!! MISMATCH on emplace: version=" << a
                        << "  key=" << b
                        << "  ESet_inserted=" << ins_es
                        << "  std::set_inserted=" << ins_ref
                        << "\n";
                }
                assert(ins_es == ins_ref);
                // 更新迭代器状态
                if (ins_es) {
                    it_version = a;
                    it = it_new;
                    valid_it = true;
                }
                break;
            }

            case 1: { // erase
                scanf("%lld%lld", &a, &b);
                if ((size_t)a >= es.size()) {
                    es.resize(a+1);
                    ref.resize(a+1);
                }
                size_t rem_es = es[a].erase(b);
                size_t rem_ref= ref[a].erase(b);
                #ifdef DEBUG
                sanity_check(es[a].root, ref[a]);
                #endif
                assert(rem_es == rem_ref);
                // 如果当前迭代器指向被删元素，则失效
                if (valid_it && it_version==a && *it==b) {
                    valid_it = false;
                }
                break;
            }

            case 2: { // copy version a → new_version
                scanf("%lld", &a);
                last_version++;
                if ((size_t)last_version >= es.size() || (size_t)a >= es.size()) {
                    size_t newsz = std::max(ll(last_version+1), a+1);
                    es.resize(newsz);
                    ref.resize(newsz);
                }
                es[last_version]  = es[a];
                ref[last_version] = ref[a];
                #ifdef DEBUG
                sanity_check(es[last_version].root, ref[last_version]);
                #endif
                break;
            }

            case 3: { // find
                scanf("%lld%lld", &a, &b);
                if ((size_t)a >= es.size()) {
                    es.resize(a+1);
                    ref.resize(a+1);
                }
                auto it2 = es[a].find(b);
                bool found_es = (it2 != es[a].end());
                bool found_ref= (ref[a].find(b) != ref[a].end());
                assert(found_es == found_ref);
                printf(found_es ? "true\n" : "false\n");
                if (found_es) {
                    it_version = a;
                    it = it2;
                    valid_it = true;
                }
                break;
            }

            case 4: { // range count
                scanf("%lld%lld%lld", &a, &b, &c);
                if ((size_t)a >= es.size()) {
                    es.resize(a+1);
                    ref.resize(a+1);
                }
                RANGE_OP_COUNT++;
                size_t ans = es[a].range(b, c);
                // 可以加一次 sanity_check 确保整个树状态无误
                if (RANGE_OP_COUNT == TARGET_RANGE_OP) {
                    std::cerr << ">>> Checking version “" << a << "” after range-op #" 
              << RANGE_OP_COUNT << "\n";
                    std::cerr << a << " " << b << " " << c << "\n";
                    sanity_check(es[a].root, ref[a]);
                     
                }
                // 同时可用 ref 计算做双重验证：
                auto it_lo = ref[a].lower_bound(b);
                auto it_hi = ref[a].upper_bound(c);
                size_t expect = distance(it_lo, it_hi);
                // if (ans != expect) {
                //     std::cerr << RANGE_OP_COUNT;
                // }
                assert(ans == expect);
                printf("%zu\n", ans);
                break;
            }

            case 5: { // prev
                if (valid_it && it != es[it_version].begin()) {
                    --it;
                    printf("%lld\n", *it);
                } else {
                    valid_it = false;
                    printf("-1\n");
                }
                // prev 并不修改数据结构，但可选择性检查
                break;
            }

            case 6: { // next
                if (valid_it) {
                    ++it;
                    if (it != es[it_version].end()) {
                        printf("%lld\n", *it);
                    } else {
                        valid_it = false;
                        printf("-1\n");
                    }
                } else {
                    printf("-1\n");
                }
                break;
            }

            default:
                // 未知操作可忽略或断言
                assert(false && "unknown op");
        }
    }
    return 0;
}