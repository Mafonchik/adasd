#pragma once

#include <fmt/core.h>

#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

template <typename Key, typename Value, typename Compare = std::less<Key>>
class Map {
public:
    Map() : root_(nullptr), cnt_(0), comp_(Compare{}) {
    }

    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;

    Map(Map&& o) noexcept : root_(o.root_), cnt_(o.cnt_), comp_(std::move(o.comp_)) {
        o.root_ = nullptr;
        o.cnt_ = 0;
    }

    Map& operator=(Map&& o) noexcept {
        if (this != &o) {
            Clear();
            root_ = o.root_;
            cnt_ = o.cnt_;
            comp_ = std::move(o.comp_);
            o.root_ = nullptr;
            o.cnt_ = 0;
        }
        return *this;
    }

    Value& operator[](const Key& key) {
        Node** p = Spot(key);
        if (*p == nullptr) {
            *p = new Node(std::pair<const Key, Value>(key, Value{}));
            ++cnt_;
        }
        return (*p)->kv_.second;
    }

    inline bool IsEmpty() const noexcept {
        return cnt_ == 0;
    }

    inline size_t Size() const noexcept {
        return cnt_;
    }

    void Swap(Map& a) noexcept {
        std::swap(root_, a.root_);
        std::swap(cnt_, a.cnt_);
        std::swap(comp_, a.comp_);
    }

    std::vector<std::pair<const Key, Value>> Values(bool is_increase = true) const noexcept {
        std::vector<std::pair<const Key, Value>> v;
        v.reserve(cnt_);
        if (is_increase) {
            GoIn(root_, v);
        } else {
            GoDe(root_, v);
        }
        return v;
    }

    void Insert(const std::pair<const Key, Value>& val) {
        Node** p = Spot(val.first);
        if (*p == nullptr) {
            *p = new Node(val);
            ++cnt_;
        } else {
            (*p)->kv_.second = val.second;
        }
    }

    void Insert(const std::initializer_list<std::pair<const Key, Value>>& values) {
        for (auto& x : values) {
            Insert(x);
        }
    }

    void Erase(const Key& key) {
        Node** p = Spot(key);
        if (*p == nullptr) {
            throw 0;
        }
        Node* x = *p;

        if (x->l_ == nullptr) {
            *p = x->r_;
            delete x;
            --cnt_;
            return;
        }
        if (x->r_ == nullptr) {
            *p = x->l_;
            delete x;
            --cnt_;
            return;
        }

        Node** q = &(x->r_);
        while ((*q)->l_) {
            q = &((*q)->l_);
        }
        Node* s = *q;

        if (q == &(x->r_)) {
            s->l_ = x->l_;
            *p = s;
            delete x;
            --cnt_;
        } else {
            *q = s->r_;
            s->l_ = x->l_;
            s->r_ = x->r_;
            *p = s;
            delete x;
            --cnt_;
        }
    }

    void Clear() noexcept {
        Drop(root_);
        root_ = nullptr;
        cnt_ = 0;
    }

    bool Find(const Key& key) const {
        Node* cur = root_;
        while (cur) {
            if (comp_(key, cur->kv_.first)) {
                cur = cur->l_;
            } else if (comp_(cur->kv_.first, key)) {
                cur = cur->r_;
            } else {
                return true;
            }
        }
        return false;
    }

    ~Map() {
        Clear();
    }

private:
    class Node {
        friend class Map;

    private:
        std::pair<const Key, Value> kv_;
        Node* l_;
        Node* r_;
        explicit Node(const std::pair<const Key, Value>& p) : kv_(p), l_(nullptr), r_(nullptr) {
        }
    };

    Node** Spot(const Key& key) {
        Node** cur = &root_;
        while (*cur) {
            if (comp_(key, (*cur)->kv_.first)) {
                cur = &((*cur)->l_);
            } else if (comp_((*cur)->kv_.first, key)) {
                cur = &((*cur)->r_);
            } else {
                break;
            }
        }
        return cur;
    }

    void GoIn(Node* t, std::vector<std::pair<const Key, Value>>& v) const {
        if (!t) {
            return;
        }
        GoIn(t->l_, v);
        v.push_back(t->kv_);
        GoIn(t->r_, v);
    }

    void GoDe(Node* t, std::vector<std::pair<const Key, Value>>& v) const {
        if (!t) {
            return;
        }
        GoDe(t->r_, v);
        v.push_back(t->kv_);
        GoDe(t->l_, v);
    }

    void Drop(Node* t) noexcept {
        if (!t) {
            return;
        }
        Drop(t->l_);
        Drop(t->r_);
        delete t;
    }

private:
    Node* root_;
    size_t cnt_;
    Compare comp_;
};
