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

        bool operator==(const MapIterator& other) const {
            return current_node_ == other.current_node_;
        }
        bool operator!=(const MapIterator& other) const {
            return current_node_ != other.current_node_;
        }
        ReferenceType operator*() const {
            return current_node_->data_;
        }
        PointerType operator->() const {
            return &current_node_->data_;
        }

        MapIterator& operator++() {
            if (!current_node_) {
                return *this;
            }
            if (current_node_->right_is_thread_) {
                current_node_ = current_node_->right_;
            } else {
                Node* temp = current_node_->right_;
                while (temp && temp->left_) {
                    temp = temp->left_;
                }
                current_node_ = temp;
            }
            return *this;
        }

        MapIterator operator++(int) {
            MapIterator temp = *this;
            ++(*this);
            return temp;
        }

    private:
        explicit MapIterator(Node* node) : current_node_(node) {
        }
        Node* current_node_{nullptr};
        friend class Map;
    };

    Map() : root_(nullptr), size_(0), comp_(Compare{}), threads_valid_(true) {
    }

    MapIterator Begin() const noexcept {
        EnsureThreaded();
        Node* current = root_;
        while (current && current->left_) {
            current = current->left_;
        }
        return MapIterator(current);
    }

    MapIterator End() const noexcept {
        EnsureThreaded();
        return MapIterator(nullptr);
    }

    Value& operator[](const Key& key) {
        Node** link = &root_;
        Node* current = root_;
        while (current) {
            if (comp_(key, current->data_.first)) {
                link = &current->left_;
                current = current->left_;
            } else if (comp_(current->data_.first, key)) {
                if (current->right_is_thread_) {
                    link = &current->right_;
                    current = nullptr;
                } else {
                    link = &current->right_;
                    current = current->right_;
                }
            } else {
                return current->data_.second;
            }
        }
        *link = new Node(std::pair<const Key, Value>(key, Value{}));
        ++size_;
        threads_valid_ = false;
        return (*link)->data_.second;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }
    size_t Size() const noexcept {
        return size_;
    }

    void Swap(Map& other) {
        static_assert(std::is_same<decltype(this->comp_), decltype(other.comp_)>::value, "Comparer mismatch");
        using std::swap;
        swap(root_, other.root_);
        swap(size_, other.size_);
        swap(comp_, other.comp_);
        threads_valid_ = false;
        other.threads_valid_ = false;
    }

    std::vector<std::pair<const Key, Value>> Values(bool increasing = true) const noexcept {
        std::vector<std::pair<const Key, Value>> values;
        values.reserve(size_);
        if (increasing) {
            TraverseInOrder(root_, values);
        } else {
            TraverseDecreasing(root_, values);
        }
        return values;
    }

    void Insert(const std::pair<const Key, Value>& value) {
        Node** link = &root_;
        Node* current = root_;
        while (current) {
            if (comp_(value.first, current->data_.first)) {
                link = &current->left_;
                current = current->left_;
            } else if (comp_(current->data_.first, value.first)) {
                if (current->right_is_thread_) {
                    link = &current->right_;
                    current = nullptr;
                } else {
                    link = &current->right_;
                    current = current->right_;
                }
            } else {
                current->data_.second = value.second;
                return;
            }
        }
        *link = new Node(value);
        ++size_;
        threads_valid_ = false;
    }

    void Insert(const std::initializer_list<std::pair<const Key, Value>>& values) {
        for (auto& value : values) {
            Insert(value);
        }
    }

    void Erase(const Key& key) {
        Node** link = &root_;
        Node* current = root_;
        while (current) {
            if (comp_(key, current->data_.first)) {
                link = &current->left_;
                current = current->left_;
            } else if (comp_(current->data_.first, key)) {
                if (current->right_is_thread_) {
                    break;
                }
                link = &current->right_;
                current = current->right_;
            } else {
                break;
            }
        }
        if (!current) {
            throw 0;
        }

        if (current->left_ == nullptr && (current->right_is_thread_ || current->right_ == nullptr)) {
            *link = nullptr;
            delete current;
            --size_;
            threads_valid_ = false;
            return;
        }

        if (current->left_ == nullptr && !current->right_is_thread_) {
            *link = current->right_;
            delete current;
            --size_;
            threads_valid_ = false;
            return;
        }

        if (current->left_ != nullptr && (current->right_is_thread_ || current->right_ == nullptr)) {
            *link = current->left_;
            delete current;
            --size_;
            threads_valid_ = false;
            return;
        }

        Node** successor_link = &current->right_;
        Node* successor = current->right_;
        while (successor->left_) {
            successor_link = &successor->left_;
            successor = successor->left_;
        }
        Node* replacement = (successor->right_is_thread_ ? nullptr : successor->right_);
        *successor_link = replacement;

        successor->left_ = current->left_;
        if (successor == current->right_) {
            successor->right_ = replacement;
        } else {
            successor->right_ = current->right_;
        }
        successor->right_is_thread_ = false;

        *link = successor;
        delete current;
        --size_;
        threads_valid_ = false;
    }

    void Clear() noexcept {
        DestroyTree(root_);
        root_ = nullptr;
        size_ = 0;
        threads_valid_ = true;
    }

    typename Map::MapIterator Find(const Key& key) const {
        EnsureThreaded();
        Node* current = root_;
        while (current) {
            if (comp_(key, current->data_.first)) {
                current = current->left_;
            } else if (comp_(current->data_.first, key)) {
                if (current->right_is_thread_) {
                    return End();
                }
                current = current->right_;
            } else {
                return MapIterator(current);
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

        std::pair<const Key, Value> data_;
        Node* left_;
        Node* right_;
        bool right_is_thread_;
        explicit Node(const std::pair<const Key, Value>& pair)
        : data_(pair), left_(nullptr), right_(nullptr), right_is_thread_(false) {
        }
    };

    static void TraverseInOrder(Node* node, std::vector<std::pair<const Key, Value>>& values) {
        if (!node) {
            return;
        }
        TraverseInOrder(node->left_, values);
        values.push_back(node->data_);
        if (!node->right_is_thread_) {
            TraverseInOrder(node->right_, values);
        }
    }

    static void TraverseDecreasing(Node* node, std::vector<std::pair<const Key, Value>>& values) {
        if (!node) {
            return;
        }
        if (!node->right_is_thread_) {
            TraverseDecreasing(node->right_, values);
        }
        values.push_back(node->data_);
        TraverseDecreasing(node->left_, values);
    }

    void ResetThreadFlags(Node* node) const {
        if (!node) {
            return;
        }
        std::vector<Node*> stack;
        Node* current = node;
        while (current || !stack.empty()) {
            while (current) {
                stack.push_back(current);
                current = current->left_;
            }
            current = stack.back();
            stack.pop_back();
            current->right_is_thread_ = false;
            current = current->right_;
        }
    }

    void BuildThreads() const {
        ResetThreadFlags(root_);
        Node* previous = nullptr;
        std::vector<Node*> stack;
        Node* current = root_;
        while (current || !stack.empty()) {
            while (current) {
                stack.push_back(current);
                current = current->left_;
            }
            current = stack.back();
            stack.pop_back();
            if (previous && previous->right_ == nullptr) {
                previous->right_ = current;
                previous->right_is_thread_ = true;
            }
            previous = current;
            current = current->right_;
        }
    }

    void EnsureThreaded() const {
        if (!threads_valid_) {
            BuildThreads();
            threads_valid_ = true;
        }
    }

    void DestroyTree(Node* node) noexcept {
        if (!node) {
            return;
        }
        DestroyTree(node->left_);
        if (!node->right_is_thread_) {
            DestroyTree(node->right_);
        }
        delete node;
    }

private:
    Node* root_;
    size_t size_;
    Compare comp_;
    mutable bool threads_valid_;
};

namespace std {
    template <typename Key, typename Value, typename Compare>
    inline void swap(Map<Key, Value, Compare>& first, Map<Key, Value, Compare>& second) {
        first.Swap(second);
    }
}  // namespace std
