#pragma once

#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

template <typename Key, typename Value, typename Compare = std::less<Key>>
class Map {
    class Node;

public:
    class MapIterator {
    public:
        using ValueType = std::pair<const Key, Value>;
        using ReferenceType = ValueType&;
        using PointerType = ValueType*;
        using DifferenceType = std::ptrdiff_t;
        using IteratorCategory = std::forward_iterator_tag;

        bool operator==(const MapIterator& o) const {
            return cur_ == o.cur_;
        }
        bool operator!=(const MapIterator& o) const {
            return cur_ != o.cur_;
        }
        ReferenceType operator*() const {
            return cur_->kv_;
        }
        PointerType operator->() const {
            return &cur_->kv_;
        }

        MapIterator& operator++() {
            if (!cur_) {
                return *this;
            }
            if (cur_->r_is_thr_) {
                cur_ = cur_->r_;
            } else {
                Node* t = cur_->r_;
                while (t && t->l_) {
                    t = t->l_;
                }
                cur_ = t;
            }
            return *this;
        }

        MapIterator operator++(int) {
            MapIterator tmp = *this;
            ++(*this);
            return tmp;
        }

    private:
        explicit MapIterator(Node* n) : cur_(n) {
        }
        Node* cur_{nullptr};
        friend class Map;
    };

    Map() : root_(nullptr), sz_(0), comp_(Compare{}), threads_ok_(true) {
    }

    MapIterator Begin() const noexcept {
        EnsureThreaded();
        Node* t = root_;
        while (t && t->l_) {
            t = t->l_;
        }
        return MapIterator(t);
    }

    MapIterator End() const noexcept {
        EnsureThreaded();
        return MapIterator(nullptr);
    }

    Value& operator[](const Key& key) {
        Node** link = &root_;
        Node* n = root_;
        while (n) {
            if (comp_(key, n->kv_.first)) {
                link = &n->l_;
                n = n->l_;
            } else if (comp_(n->kv_.first, key)) {
                if (n->r_is_thr_) {
                    link = &n->r_;
                    n = nullptr;
                } else {
                    link = &n->r_;
                    n = n->r_;
                }
            } else {
                return n->kv_.second;
            }
        }
        *link = new Node(std::pair<const Key, Value>(key, Value{}));
        ++sz_;
        threads_ok_ = false;
        return (*link)->kv_.second;
    }

    bool IsEmpty() const noexcept {
        return sz_ == 0;
    }
    size_t Size() const noexcept {
        return sz_;
    }

    void Swap(Map& a) {
        static_assert(std::is_same<decltype(this->comp_), decltype(a.comp_)>::value, "Comparer mismatch");
        using std::swap;
        swap(root_, a.root_);
        swap(sz_, a.sz_);
        swap(comp_, a.comp_);
        threads_ok_ = false;
        a.threads_ok_ = false;
    }

    std::vector<std::pair<const Key, Value>> Values(bool inc = true) const noexcept {
        std::vector<std::pair<const Key, Value>> v;
        v.reserve(sz_);
        if (inc) {
            GoIn(root_, v);
        } else {
            GoDe(root_, v);
        }
        return v;
    }

    void Insert(const std::pair<const Key, Value>& val) {
        Node** link = &root_;
        Node* n = root_;
        while (n) {
            if (comp_(val.first, n->kv_.first)) {
                link = &n->l_;
                n = n->l_;
            } else if (comp_(n->kv_.first, val.first)) {
                if (n->r_is_thr_) {
                    link = &n->r_;
                    n = nullptr;
                } else {
                    link = &n->r_;
                    n = n->r_;
                }
            } else {
                n->kv_.second = val.second;
                return;
            }
        }
        *link = new Node(val);
        ++sz_;
        threads_ok_ = false;
    }

    void Insert(const std::initializer_list<std::pair<const Key, Value>>& values) {
        for (auto& x : values) {
            Insert(x);
        }
    }

    void Erase(const Key& key) {
        Node** link = &root_;
        Node* n = root_;
        while (n) {
            if (comp_(key, n->kv_.first)) {
                link = &n->l_;
                n = n->l_;
            } else if (comp_(n->kv_.first, key)) {
                if (n->r_is_thr_) {
                    break;
                }
                link = &n->r_;
                n = n->r_;
            } else {
                break;
            }
        }
        if (!n) {
            throw 0;
        }

        if (n->l_ == nullptr && (n->r_is_thr_ || n->r_ == nullptr)) {
            *link = nullptr;
            delete n;
            --sz_;
            threads_ok_ = false;
            return;
        }

        if (n->l_ == nullptr && !n->r_is_thr_) {
            *link = n->r_;
            delete n;
            --sz_;
            threads_ok_ = false;
            return;
        }

        if (n->l_ != nullptr && (n->r_is_thr_ || n->r_ == nullptr)) {
            *link = n->l_;
            delete n;
            --sz_;
            threads_ok_ = false;
            return;
        }

        Node** slink = &n->r_;
        Node* s = n->r_;
        while (s->l_) {
            slink = &s->l_;
            s = s->l_;
        }
        Node* rep = (s->r_is_thr_ ? nullptr : s->r_);
        *slink = rep;

        s->l_ = n->l_;
        if (s == n->r_) {
            s->r_ = rep;
        } else {
            s->r_ = n->r_;
        }
        s->r_is_thr_ = false;

        *link = s;
        delete n;
        --sz_;
        threads_ok_ = false;
    }

    void Clear() noexcept {
        Drop(root_);
        root_ = nullptr;
        sz_ = 0;
        threads_ok_ = true;
    }

    typename Map::MapIterator Find(const Key& key) const {
        EnsureThreaded();
        Node* n = root_;
        while (n) {
            if (comp_(key, n->kv_.first)) {
                n = n->l_;
            } else if (comp_(n->kv_.first, key)) {
                if (n->r_is_thr_) {
                    return End();
                }
                n = n->r_;
            } else {
                return MapIterator(n);
            }
        }
        return End();
    }

    ~Map() {
        Clear();
    }

private:
    class Node {
        friend class MapIterator;
        friend class Map;

        std::pair<const Key, Value> kv_;
        Node* l_;
        Node* r_;
        bool r_is_thr_;
        explicit Node(const std::pair<const Key, Value>& p) : kv_(p), l_(nullptr), r_(nullptr), r_is_thr_(false) {
        }
    };

    static void GoIn(Node* t, std::vector<std::pair<const Key, Value>>& v) {
        if (!t) {
            return;
        }
        GoIn(t->l_, v);
        v.push_back(t->kv_);
        if (!t->r_is_thr_) {
            GoIn(t->r_, v);
        }
    }

    static void GoDe(Node* t, std::vector<std::pair<const Key, Value>>& v) {
        if (!t) {
            return;
        }
        if (!t->r_is_thr_) {
            GoDe(t->r_, v);
        }
        v.push_back(t->kv_);
        GoDe(t->l_, v);
    }

    void ResetThreadFlags(Node* t) const {
        if (!t) {
            return;
        }
        std::vector<Node*> st;
        Node* cur = t;
        while (cur || !st.empty()) {
            while (cur) {
                st.push_back(cur);
                cur = cur->l_;
            }
            cur = st.back();
            st.pop_back();
            cur->r_is_thr_ = false;
            cur = cur->r_;
        }
    }

    void BuildThreads() const {
        ResetThreadFlags(root_);
        Node* prev = nullptr;
        std::vector<Node*> st;
        Node* cur = root_;
        while (cur || !st.empty()) {
            while (cur) {
                st.push_back(cur);
                cur = cur->l_;
            }
            cur = st.back();
            st.pop_back();
            if (prev && prev->r_ == nullptr) {
                prev->r_ = cur;
                prev->r_is_thr_ = true;
            }
            prev = cur;
            cur = cur->r_;
        }
    }

    void EnsureThreaded() const {
        if (!threads_ok_) {
            BuildThreads();
            threads_ok_ = true;
        }
    }

    void Drop(Node* t) noexcept {
        if (!t) {
            return;
        }
        Drop(t->l_);
        if (!t->r_is_thr_) {
            Drop(t->r_);
        }
        delete t;
    }

private:
    Node* root_;
    size_t sz_;
    Compare comp_;
    mutable bool threads_ok_;
};

namespace std {
template <typename Key, typename Value, typename Compare>
inline void swap(Map<Key, Value, Compare>& a, Map<Key, Value, Compare>& b) {  // NOLINT(readability-identifier-naming)
    a.Swap(b);
}
}  // namespace std
