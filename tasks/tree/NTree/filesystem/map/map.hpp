#pragma once

#include <fmt/core.h>

#include <cstdlib>
#include <functional>
#include <utility>
#include <vector>

template <typename Key, typename Value, typename Compare = std::less<Key>>
class Map {
private:
    class Node {
        friend class Map;

    private:
        Key key_;
        Value value_;
        Node* left_ = nullptr;
        Node* right_ = nullptr;
        explicit Node(const Key& k, const Value& v) : key_(k), value_(v), left_(nullptr), right_(nullptr) {
        }
    };

public:
    Map() : root_(nullptr), size_(0), comp_() {
    }

    Value& operator[](const Key& key) {
        Node** current = &root_;
        while (*current != nullptr) {
            if (comp_(key, (*current)->key_)) {
                current = &(*current)->left_;
            } else if (comp_((*current)->key_, key)) {
                current = &(*current)->right_;
            } else {
                return (*current)->value_;
            }
        }
        *current = new Node(key, Value());
        size_++;
        return (*current)->value_;
    }

    inline bool IsEmpty() const noexcept {
        return root_ == nullptr;
    }

    inline size_t Size() const noexcept {
        return size_;
    }

    void Swap(Map& other) {
        static_assert(std::is_same<decltype(this->comp_), decltype(other.comp_)>::value,
                      "The compare function types are different");
        std::swap(root_, other.root_);
        std::swap(size_, other.size_);
        std::swap(comp_, other.comp_);
    }

    std::vector<std::pair<const Key, Value>> Values(bool is_increase = true) const noexcept {
        std::vector<std::pair<const Key, Value>> result;
        result.reserve(size_);
        if (is_increase) {
            InOrderTraversal(root_, result);
        } else {
            ReverseInOrderTraversal(root_, result);
        }
        return result;
    }

    void Insert(const std::pair<const Key, Value>& val) {
        Node** current = &root_;
        while (*current != nullptr) {
            if (comp_(val.first, (*current)->key_)) {
                current = &(*current)->left_;
            } else if (comp_((*current)->key_, val.first)) {
                current = &(*current)->right_;
            } else {
                (*current)->value_ = val.second;
                return;
            }
        }
        *current = new Node(val.first, val.second);
        size_++;
    }

    void Insert(const std::initializer_list<std::pair<const Key, Value>>& values) {
        for (const auto& val : values) {
            Insert(val);
        }
    }

    void Erase(const Key& key) {
        Node* parent = nullptr;
        Node* current = root_;

        while (current != nullptr) {
            if (comp_(key, current->key_)) {
                parent = current;
                current = current->left_;
            } else if (comp_(current->key_, key)) {
                parent = current;
                current = current->right_;
            } else {
                break;
            }
        }

        if (current == nullptr) {
            throw 1;
        }

        if (current->left_ == nullptr && current->right_ == nullptr) {
            if (parent == nullptr) {
                root_ = nullptr;
            } else if (parent->left_ == current) {
                parent->left_ = nullptr;
            } else {
                parent->right_ = nullptr;
            }
            delete current;
        } else if (current->left_ == nullptr || current->right_ == nullptr) {
            Node* child = (current->left_ != nullptr) ? current->left_ : current->right_;
            if (parent == nullptr) {
                root_ = child;
            } else if (parent->left_ == current) {
                parent->left_ = child;
            } else {
                parent->right_ = child;
            }
            delete current;
        } else {
            Node* min_parent = current;
            Node* min_node = current->right_;
            while (min_node->left_ != nullptr) {
                min_parent = min_node;
                min_node = min_node->left_;
            }
            current->key_ = min_node->key_;
            current->value_ = min_node->value_;
            if (min_parent->left_ == min_node) {
                min_parent->left_ = min_node->right_;
            } else {
                min_parent->right_ = min_node->right_;
            }
            delete min_node;
        }
        size_--;
    }

    void Clear() noexcept {
        ClearTree(root_);
        root_ = nullptr;
        size_ = 0;
    }

    bool Find(const Key& key) const {
        Node* current = root_;
        while (current != nullptr) {
            if (comp_(key, current->key_)) {
                current = current->left_;
            } else if (comp_(current->key_, key)) {
                current = current->right_;
            } else {
                return true;
            }
        }
        return false;
    }

private:
    void ClearTree(Node* node) {
        if (node == nullptr) {
            return;
        }
        ClearTree(node->left_);
        ClearTree(node->right_);
        delete node;
    }

    void InOrderTraversal(Node* node, std::vector<std::pair<const Key, Value>>& result) const {
        if (node == nullptr) {
            return;
        }
        InOrderTraversal(node->left_, result);
        result.emplace_back(node->key_, node->value_);
        InOrderTraversal(node->right_, result);
    }

    void ReverseInOrderTraversal(Node* node, std::vector<std::pair<const Key, Value>>& result) const {
        if (node == nullptr) {
            return;
        }
        ReverseInOrderTraversal(node->right_, result);
        result.emplace_back(node->key_, node->value_);
        ReverseInOrderTraversal(node->left_, result);
    }

public:
    ~Map() {
        Clear();
    }

private:
    Node* root_ = nullptr;
    size_t size_ = 0;
    Compare comp_;
};

namespace std {
// Global swap overloading
template <typename Key, typename Value>
// NOLINTNEXTLINE
void swap(Map<Key, Value>& a, Map<Key, Value>& b) {
    a.Swap(b);
}
}  // namespace std
