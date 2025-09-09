#pragma once

#include <fmt/core.h>

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "exceptions.hpp"

template <typename T>
class List {
private:
    class Node {
        friend class List;
        friend class ListIterator;

        T value_{};
        Node* prev_{nullptr};
        Node* next_{nullptr};

        explicit Node(const T& val, Node* prev = nullptr, Node* next = nullptr)
            : value_(val), prev_(prev), next_(next) {
        }
        explicit Node(T&& val, Node* prev = nullptr, Node* next = nullptr)
            : value_(std::move(val)), prev_(prev), next_(next) {
        }
    };

public:
    class ListIterator {
    public:
        using value_type = T;
        using reference = value_type&;
        using pointer = value_type*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        ListIterator() : current_(nullptr), tail_hint_(nullptr) {
        }

        bool operator==(const ListIterator& other) const {
            return current_ == other.current_;
        }

        bool operator!=(const ListIterator& other) const {
            return current_ != other.current_;
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

        // ++it
        ListIterator& operator++() {
            if (current_ != nullptr) {
                current_ = current_->next_;
            }
            // если уже end(), остаёмся end()
            return *this;
        }

        // it++
        ListIterator operator++(int) {
            ListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        // --it
        ListIterator& operator--() {
            if (current_ != nullptr) {
                current_ = current_->prev_;
            } else {
                // шаг назад от end() -> на хвост
                current_ = tail_hint_;
            }
            return *this;
        }

        // it--
        ListIterator operator--(int) {
            ListIterator tmp = *this;
            --(*this);
            return tmp;
        }

    private:
        explicit ListIterator(Node* node, Node* tail_hint) : current_(node), tail_hint_(tail_hint) {
        }

        Node* current_{nullptr};
        Node* tail_hint_{nullptr};

        friend class List;
    };

public:
    List() : head_(nullptr), tail_(nullptr), size_(0) {
    }

    explicit List(size_t sz) : head_(nullptr), tail_(nullptr), size_(0) {
        for (size_t i = 0; i < sz; ++i) {
            PushBack(T{});
        }
    }

    List(const std::initializer_list<T>& values) : head_(nullptr), tail_(nullptr), size_(0) {
        for (const auto& v : values) {
            PushBack(v);
        }
    }

    List(const List& other) : head_(nullptr), tail_(nullptr), size_(0) {
        for (Node* cur = other.head_; cur != nullptr; cur = cur->next_) {
            PushBack(cur->value_);
        }
    }

    List& operator=(const List& other) {
        if (this != &other) {
            Clear();
            for (Node* cur = other.head_; cur != nullptr; cur = cur->next_) {
                PushBack(cur->value_);
            }
        }
        return *this;
    }

    ListIterator Begin() const noexcept {
        return ListIterator(head_, tail_);
    }

    ListIterator End() const noexcept {
        // end() — это «после хвоста», current_=nullptr, но мы передаём tail_ как подсказку
        return ListIterator(nullptr, tail_);
    }

    T& Front() const {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty");
        }
        return head_->value_;
    }

    T& Back() const {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty");
        }
        return tail_->value_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }
    size_t Size() const noexcept {
        return size_;
    }

    void Swap(List& other) {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    ListIterator Find(const T& value) const {
        for (Node* cur = head_; cur != nullptr; cur = cur->next_) {
            if (cur->value_ == value) {
                return ListIterator(cur, tail_);
            }
        }
        return End();
    }

    void Erase(ListIterator pos) {
        Node* n = pos.current_;
        if (!n)
            return;  // удалять end() — no-op

        if (n->prev_)
            n->prev_->next_ = n->next_;
        else
            head_ = n->next_;

        if (n->next_)
            n->next_->prev_ = n->prev_;
        else
            tail_ = n->prev_;

        delete n;
        --size_;
    }

    void Insert(ListIterator pos, const T& value) {
        // вставка ДО pos
        if (pos.current_ == nullptr) {  // вставка перед end() => push_back
            PushBack(value);
            return;
        }

        Node* at = pos.current_;
        Node* left = at->prev_;
        Node* nn = new Node(value, left, at);
        at->prev_ = nn;
        if (left) {
            left->next_ = nn;
        } else {
            head_ = nn;
        }
        ++size_;
    }

    void Clear() noexcept {
        Node* cur = head_;
        while (cur) {
            Node* nxt = cur->next_;
            delete cur;
            cur = nxt;
        }
        head_ = tail_ = nullptr;
        size_ = 0;
    }

    void PushBack(const T& value) {
        Node* nn = new Node(value, tail_, nullptr);
        if (tail_) {
            tail_->next_ = nn;
        } else {
            head_ = nn;
        }
        tail_ = nn;
        ++size_;
    }

    void PushFront(const T& value) {
        Node* nn = new Node(value, nullptr, head_);
        if (head_) {
            head_->prev_ = nn;
        } else {
            tail_ = nn;
        }
        head_ = nn;
        ++size_;
    }

    void PopBack() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty");
        }
        Node* n = tail_;
        tail_ = tail_->prev_;
        if (tail_)
            tail_->next_ = nullptr;
        else
            head_ = nullptr;
        delete n;
        --size_;
    }

    void PopFront() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty");
        }
        Node* n = head_;
        head_ = head_->next_;
        if (head_)
            head_->prev_ = nullptr;
        else
            tail_ = nullptr;
        delete n;
        --size_;
    }

    ~List() {
        Clear();
    }

private:
    Node* head_{nullptr};
    Node* tail_{nullptr};
    size_t size_{0};
};

namespace std {
template <typename T>
// NOLINTNEXTLINE
void swap(List<T>& a, List<T>& b) {
    a.Swap(b);
}
}  // namespace std
