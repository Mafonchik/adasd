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
class ForwardList {
private:
    class Node {
        friend class ForwardList;
        friend class ListIterator;

        T value_{};
        Node* next_{nullptr};

        explicit Node(const T& val, Node* next = nullptr) : value_(val), next_(next) {
        }
        explicit Node(T&& val, Node* next = nullptr) : value_(std::move(val)), next_(next) {
        }

        // Удаляем нежелательные операции
        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;
    };

public:
    class ListIterator {
    public:
        using value_type = T;
        using reference = value_type&;
        using pointer = value_type*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        ListIterator() : current_(nullptr), list_(nullptr) {
        }

        ListIterator(const ListIterator& other) = default;
        ListIterator& operator=(const ListIterator& other) = default;

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
            return *this;
        }

        // it++
        ListIterator operator++(int) {
            ListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

    private:
        explicit ListIterator(Node* node, const ForwardList* list) : current_(node), list_(list) {
        }

        Node* current_{nullptr};
        const ForwardList* list_{nullptr};

        friend class ForwardList;
    };

public:
    ForwardList() : head_(nullptr), tail_(nullptr), size_(0) {
    }

    explicit ForwardList(size_t sz) : head_(nullptr), tail_(nullptr), size_(0) {
        try {
            for (size_t i = 0; i < sz; ++i) {
                PushBack(T());
            }
        } catch (...) {
            Clear();
            throw;
        }
    }

    ForwardList(const std::initializer_list<T>& values) : head_(nullptr), tail_(nullptr), size_(0) {
        try {
            for (const auto& v : values) {
                PushBack(v);
            }
        } catch (...) {
            Clear();
            throw;
        }
    }

    ForwardList(const ForwardList& other) : head_(nullptr), tail_(nullptr), size_(0) {
        try {
            for (Node* cur = other.head_; cur != nullptr; cur = cur->next_) {
                PushBack(cur->value_);
            }
        } catch (...) {
            Clear();
            throw;
        }
    }

    ForwardList(ForwardList&& other) noexcept : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    ForwardList& operator=(const ForwardList& other) {
        if (this == &other) {
            return *this;
        }

        ForwardList temp(other);
        Swap(temp);
        return *this;
    }

    ForwardList& operator=(ForwardList&& other) noexcept {
        if (this != &other) {
            Clear();
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;

            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ListIterator BeforeBegin() const noexcept {
        return ListIterator(nullptr, this);
    }

    ListIterator Begin() const noexcept {
        return ListIterator(head_, this);
    }

    ListIterator End() const noexcept {
        return ListIterator(nullptr, this);
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

    void Swap(ForwardList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    ListIterator Find(const T& value) const {
        for (Node* cur = head_; cur != nullptr; cur = cur->next_) {
            if (cur->value_ == value) {
                return ListIterator(cur, this);
            }
        }
        return End();
    }

    void EraseAfter(ListIterator pos) {
        if (pos.list_ != this) {
            throw std::runtime_error("Iterator does not belong to this list");
        }

        Node* current = pos.current_;
        if (current == nullptr) {
            // EraseAfter(BeforeBegin()) - удаляем первый элемент
            if (head_) {
                Node* to_delete = head_;
                head_ = head_->next_;
                if (tail_ == to_delete) {
                    tail_ = nullptr;
                }
                delete to_delete;
                --size_;
            }
            return;
        }

        if (current->next_ == nullptr) {
            return;  // Нет элемента после текущего
        }

        Node* to_delete = current->next_;
        current->next_ = to_delete->next_;

        if (to_delete == tail_) {
            tail_ = current;
        }

        delete to_delete;
        --size_;
    }

    ListIterator InsertAfter(ListIterator pos, const T& value) {
        if (pos.list_ != this) {
            throw std::runtime_error("Iterator does not belong to this list");
        }

        Node* current = pos.current_;
        if (current == nullptr) {
            // InsertAfter(BeforeBegin()) - вставка в начало
            PushFront(value);
            return Begin();
        }

        Node* new_node = new Node(value, current->next_);
        current->next_ = new_node;

        if (current == tail_) {
            tail_ = new_node;
        }

        ++size_;
        return ListIterator(new_node, this);
    }

    void Erase(ListIterator pos) {
        if (pos.list_ != this) {
            throw std::runtime_error("Iterator does not belong to this list");
        }

        Node* n = pos.current_;
        if (!n)
            return;  // удалять end() — no-op

        // Находим предыдущий узел
        Node* prev = nullptr;
        Node* cur = head_;
        while (cur && cur != n) {
            prev = cur;
            cur = cur->next_;
        }

        if (prev) {
            prev->next_ = n->next_;
        } else {
            head_ = n->next_;
        }

        if (n == tail_) {
            tail_ = prev;
        }

        delete n;
        --size_;
    }

    ListIterator Insert(ListIterator pos, const T& value) {
        if (pos.list_ != this) {
            throw std::runtime_error("Iterator does not belong to this list");
        }

        // вставка ДО pos
        if (pos.current_ == nullptr) {  // вставка перед end() => push_back
            PushBack(value);
            return ListIterator(tail_, this);
        }

        Node* at = pos.current_;

        // Находим предыдущий узел
        Node* prev = nullptr;
        Node* cur = head_;
        while (cur && cur != at) {
            prev = cur;
            cur = cur->next_;
        }

        Node* nn = new Node(value, at);

        if (prev) {
            prev->next_ = nn;
        } else {
            head_ = nn;
        }

        ++size_;
        return ListIterator(nn, this);
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
        Node* nn = new Node(value);
        if (tail_) {
            tail_->next_ = nn;
        } else {
            head_ = nn;
        }
        tail_ = nn;
        ++size_;
    }

    void PushFront(const T& value) {
        Node* nn = new Node(value, head_);
        head_ = nn;
        if (!tail_) {
            tail_ = nn;
        }
        ++size_;
    }

    void PopBack() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty");
        }

        if (head_ == tail_) {
            delete head_;
            head_ = tail_ = nullptr;
        } else {
            // Находим предпоследний узел
            Node* prev = nullptr;
            Node* cur = head_;
            while (cur->next_) {
                prev = cur;
                cur = cur->next_;
            }

            prev->next_ = nullptr;
            tail_ = prev;
            delete cur;
        }
        --size_;
    }

    void PopFront() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty");
        }
        Node* n = head_;
        head_ = head_->next_;
        if (!head_) {
            tail_ = nullptr;
        }
        delete n;
        --size_;
    }

    ~ForwardList() {
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
void swap(ForwardList<T>& a, ForwardList<T>& b) noexcept {
    a.Swap(b);
}
}  // namespace std
