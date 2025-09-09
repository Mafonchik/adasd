#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>

class ListIsEmptyException : public std::exception {
public:
    explicit ListIsEmptyException(const std::string& text = "List is empty") : error_message_(text) {
    }

    const char* what() const noexcept override {
        return error_message_.c_str();
    }

private:
    std::string error_message_;
};

template <typename T>
class ForwardList {
private:
    class Node {
        friend class ForwardListIterator;
        friend class ForwardList;

    private:
        T value_;
        Node* next_;

        explicit Node(const T& val, Node* nxt = nullptr) : value_(val), next_(nxt) {
        }
        explicit Node(T&& val, Node* nxt = nullptr) : value_(std::move(val)), next_(nxt) {
        }
    };

public:
    class ForwardListIterator {
    public:
        using iterator_category = std::forward_iterator_tag;  // NOLINT
        using value_type = T;                                 // NOLINT
        using difference_type = std::ptrdiff_t;               // NOLINT
        using pointer = T*;                                   // NOLINT
        using reference = T&;                                 // NOLINT

        ForwardListIterator() : current_(nullptr) {
        }

        reference operator*() const {
            if (current_ == nullptr) {
                throw std::runtime_error("Dereferencing end iterator");
            }
            return current_->value_;
        }

        pointer operator->() const {
            if (current_ == nullptr) {
                throw std::runtime_error("Dereferencing end iterator");
            }
            return &current_->value_;
        }

        ForwardListIterator& operator++() {
            if (current_ != nullptr) {
                current_ = current_->next_;
            }
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
        explicit ForwardListIterator(Node* node) : current_(node) {
        }

    private:
        Node* current_;

        friend class ForwardList;
    };

public:
    ForwardList() : head_(nullptr), size_(0) {
    }

    explicit ForwardList(size_t sz) : head_(nullptr), size_(0) {
        for (size_t i = 0; i < sz; ++i) {
            PushFront(T());
        }
    }

    ForwardList(const std::initializer_list<T>& values) : head_(nullptr), size_(0) {
        for (auto it = values.end(); it != values.begin();) {
            --it;
            PushFront(*it);
        }
    }

    ForwardList(const ForwardList& other) : head_(nullptr), size_(0) {
        if (!other.IsEmpty()) {
            ForwardList temp;
            Node* other_current = other.head_;
            while (other_current != nullptr) {
                temp.PushFront(other_current->value_);
                other_current = other_current->next_;
            }

            while (!temp.IsEmpty()) {
                PushFront(temp.Front());
                temp.PopFront();
            }
        }
    }

    ForwardList& operator=(const ForwardList& other) {
        if (this != &other) {
            Clear();
            if (!other.IsEmpty()) {
                ForwardList temp;
                Node* other_current = other.head_;
                while (other_current != nullptr) {
                    temp.PushFront(other_current->value_);
                    other_current = other_current->next_;
                }

                while (!temp.IsEmpty()) {
                    PushFront(temp.Front());
                    temp.PopFront();
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
            throw ListIsEmptyException("List is empty");
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
        size_--;
    }

    void InsertAfter(ForwardListIterator pos, const T& value) {
        if (pos.current_ == nullptr) {
            PushFront(value);
            return;
        }

        Node* new_node = new Node(value, pos.current_->next_);
        pos.current_->next_ = new_node;
        size_++;
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
        Node* current = head_;
        while (current != nullptr) {
            Node* next = current->next_;
            delete current;
            current = next;
        }
        head_ = nullptr;
        size_ = 0;
    }

    void PushFront(const T& value) {
        head_ = new Node(value, head_);
        size_++;
    }

    void PushFront(T&& value) {
        head_ = new Node(std::move(value), head_);
        size_++;
    }

    void PopFront() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty");
        }

        Node* to_delete = head_;
        head_ = head_->next_;
        delete to_delete;
        size_--;
    }

    ~ForwardList() {
        Clear();
    }

private:
    Node* head_;
    size_t size_;
};

namespace std {
template <typename T>
void swap(ForwardList<T>& a, ForwardList<T>& b) {  // NOLINT
    a.Swap(b);
}
}  // namespace std
