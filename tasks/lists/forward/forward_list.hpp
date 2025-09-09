#pragma once

#include <fmt/core.h>

#include <cstdlib>
#include <functional>
#include <iterator>
#include <utility>

template <typename T>
class ForwardList {
private:
    class Node {
        friend class ForwardListIterator;
        friend class ForwardList;

    private:
        T value_;
        Node* next_;

        explicit Node(const T& value, Node* next = nullptr)
            : value_(value), next_(next) {}
    };

public:
    class ForwardListIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        ForwardListIterator() : current_(nullptr) {}
        
        reference operator*() const {
            return current_->value_;
        }
        
        pointer operator->() const {
            return &current_->value_;
        }
        
        ForwardListIterator& operator++() {
            current_ = current_->next_;
            return *this;
        }
        
        ForwardListIterator operator++(int) {
            ForwardListIterator temp = *this;
            ++(*this);
            return temp;
        }
        
        bool operator==(const ForwardListIterator& other) const {
            return current_ == other.current_;
        }
        
        bool operator!=(const ForwardListIterator& other) const {
            return !(*this == other);
        }

    private:
        explicit ForwardListIterator(Node* node) : current_(node) {}

    private:
        Node* current_;
        
        friend class ForwardList;
    };

public:
    ForwardList() : head_(nullptr), size_(0) {}

    explicit ForwardList(size_t sz) : head_(nullptr), size_(0) {
        for (size_t i = 0; i < sz; ++i) {
            PushFront(T());
        }
    }

    ForwardList(const std::initializer_list<T>& values) : head_(nullptr), size_(0) {
        for (auto it = values.end(); it != values.begin();) {
            PushFront(*--it);
        }
    }

    ForwardList(const ForwardList& other) : head_(nullptr), size_(0) {
        if (!other.IsEmpty()) {
            Node* other_current = other.head_;
            Node* new_head = new Node(other_current->value_);
            head_ = new_head;
            Node* current = head_;
            size_ = 1;
            
            other_current = other_current->next_;
            while (other_current != nullptr) {
                current->next_ = new Node(other_current->value_);
                current = current->next_;
                other_current = other_current->next_;
                ++size_;
            }
        }
    }

    ForwardList& operator=(const ForwardList& other) {
        if (this != &other) {
            Clear();
            if (!other.IsEmpty()) {
                Node* other_current = other.head_;
                Node* new_head = new Node(other_current->value_);
                head_ = new_head;
                Node* current = head_;
                size_ = 1;
                
                other_current = other_current->next_;
                while (other_current != nullptr) {
                    current->next_ = new Node(other_current->value_);
                    current = current->next_;
                    other_current = other_current->next_;
                    ++size_;
                }
            }
        }
        return *this;
    }

    ForwardListIterator Begin() const noexcept {
        return ForwardListIterator(head_);
    }

    ForwardListIterator End() const noexcept {
        return ForwardListIterator(nullptr);
    }

    inline T& Front() const {
        if (IsEmpty()) {
            throw std::out_of_range("List is empty");
        }
        return head_->value_;
    }

    inline bool IsEmpty() const noexcept {
        return head_ == nullptr;
    }

    inline size_t Size() const noexcept {
        return size_;
    }

    void Swap(ForwardList& other) {
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
    }

    void EraseAfter(ForwardListIterator pos) {
        if (pos.current_ == nullptr || pos.current_->next_ == nullptr) {
            return;
        }
        
        Node* to_delete = pos.current_->next_;
        pos.current_->next_ = to_delete->next_;
        delete to_delete;
        --size_;
    }

    void InsertAfter(ForwardListIterator pos, const T& value) {
        if (pos.current_ == nullptr) {
            PushFront(value);
            return;
        }
        
        Node* new_node = new Node(value, pos.current_->next_);
        pos.current_->next_ = new_node;
        ++size_;
    }

    ForwardListIterator Find(const T& value) const {
        Node* current = head_;
        while (current != nullptr) {
            if (current->value_ == value) {
                return ForwardListIterator(current);
            }
            current = current->next_;
        }
        return End();
    }

    void Clear() noexcept {
        while (!IsEmpty()) {
            PopFront();
        }
    }

    void PushFront(const T& value) {
        Node* new_node = new Node(value, head_);
        head_ = new_node;
        ++size_;
    }

    void PopFront() {
        if (IsEmpty()) {
            throw std::out_of_range("List is empty");
        }
        
        Node* to_delete = head_;
        head_ = head_->next_;
        delete to_delete;
        --size_;
    }

    ~ForwardList() {
        Clear();
    }

private:
    Node* head_;
    size_t size_;
};

namespace std {
// Global swap overloading
template <typename T>
void swap(ForwardList<T>& a, ForwardList<T>& b) {  // NOLINT
    a.Swap(b);
}
}  // namespace std
