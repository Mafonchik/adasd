#include "vector.hpp"

#include <memory>

template <typename T>
Vector<T>::Vector() {
}

template <typename T>
Vector<T>::Vector(size_t count, const T& value) {
    if (count > 0) {
        data_ = static_cast<T*>(operator new(count * sizeof(T)));
        size_ = count;
        capacity_ = count;
        for (size_t i = 0; i < count; ++i) {
            new (data_ + i) T(value);
        }
    }
}

template <typename T>
Vector<T>::Vector(const Vector& other) {
    if (other.size_ > 0) {
        data_ = static_cast<T*>(operator new(other.size_ * sizeof(T)));
        size_ = other.size_;
        capacity_ = other.size_;
        std::uninitialized_copy(other.data_, other.data_ + other.size_, data_);
    }
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& other) {
    if (this != &other) {
        Vector<T> temp(other);
        std::swap(data_, temp.data_);
        std::swap(size_, temp.size_);
        std::swap(capacity_, temp.capacity_);
    }
    return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& other) {
    if (this != &other) {
        this->~Vector();
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    return *this;
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

template <typename T>
Vector<T>::Vector(std::initializer_list<T> init) {
    if (init.size() > 0) {
        data_ = static_cast<T*>(operator new(init.size() * sizeof(T)));
        size_ = init.size();
        capacity_ = init.size() + 1;
        std::uninitialized_copy(init.begin(), init.end(), data_);
    }
}

template <typename T>
T& Vector<T>::operator[](size_t pos) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pos >= size_) {
        throw std::out_of_range("Index out of range");
    }
    return data_[pos];
}

template <typename T>
T& Vector<T>::Front() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_[0];
}

template <typename T>
bool Vector<T>::IsEmpty() const noexcept {
    return size_ == 0;
}

template <typename T>
T& Vector<T>::Back() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_[size_ - 1];
}

template <typename T>
T* Vector<T>::Data() const noexcept {
    return data_;
}

template <typename T>
size_t Vector<T>::Size() const noexcept {
    return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const noexcept {
    return capacity_;
}

template <typename T>
void Vector<T>::Reserve(size_t new_cap) {
    if (new_cap > capacity_) {
        Reallocate(new_cap);
    }
}

template <typename T>
void Vector<T>::Clear() noexcept {
    if constexpr (std::is_same_v<T, void*>) {
        for (size_t i = 0; i < size_; ++i) {
            free(data_[i]);
        }
    }
    for (size_t i = 0; i < size_; ++i) {
        data_[i].~T();
    }
    size_ = 0;
}

template <typename T>
void Vector<T>::Insert(size_t pos, T value) {
    if (pos > size_) {
        throw std::out_of_range("Insert position out of range");
    }
    if (size_ == capacity_) {
        Reallocate(capacity_ == 0 ? DefaultCapacity : capacity_ * 2);
    }
    if (pos < size_) {
        new (data_ + size_) T(std::move(data_[size_ - 1]));
        for (size_t i = size_; i > pos; --i) {
            data_[i] = std::move(data_[i - 1]);
        }
        data_[pos] = std::move(value);
    } else {
        new (data_ + size_) T(std::move(value));
    }
    ++size_;
}

template <typename T>
void Vector<T>::Erase(size_t begin_pos, size_t end_pos) {
    if (begin_pos >= end_pos || end_pos > size_) {
        return;
    }
    for (size_t i = begin_pos; i + (end_pos - begin_pos) < size_; ++i) {
        data_[i] = std::move(data_[i + (end_pos - begin_pos)]);
    }
    for (size_t i = size_ - (end_pos - begin_pos); i < size_; ++i) {
        data_[i].~T();
    }
    size_ -= (end_pos - begin_pos);
}

template <typename T>
void Vector<T>::PushBack(T value) {
    if (size_ == capacity_) {
        Reallocate(capacity_ == 0 ? DefaultCapacity : capacity_ * 2);
    }
    new (data_ + size_) T(std::move(value));
    ++size_;
}

template <typename T>
template <class... Args>
void Vector<T>::EmplaceBack(Args&&... args) {
    if (size_ == capacity_) {
        Reallocate(capacity_ == 0 ? DefaultCapacity : capacity_ * 2);
    }
    new (data_ + size_) T(std::forward<Args>(args)...);
    ++size_;
}

template <typename T>
void Vector<T>::PopBack() {
    if (size_ > 0) {
        data_[size_ - 1].~T();
        --size_;
    }
}

template <typename T>
void Vector<T>::Resize(size_t count, const T& value) {
    if (count > capacity_) {
        Reallocate(count);
    }
    if (count > size_) {
        for (size_t i = size_; i < count; ++i) {
            new (data_ + i) T(value);
        }
    } else if (count < size_) {
        for (size_t i = count; i < size_; ++i) {
            data_[i].~T();
        }
    }
    size_ = count;
}

template <typename T>
Vector<T>::~Vector() {
    Clear();
    operator delete(data_);
}

template <typename T>
void Vector<T>::Reallocate(size_t new_cap) {
    T* new_data = static_cast<T*>(operator new(new_cap * sizeof(T)));
    for (size_t i = 0; i < size_; ++i) {
        new (new_data + i) T(std::move(data_[i]));
        data_[i].~T();
    }
    operator delete(data_);
    data_ = new_data;
    capacity_ = new_cap;
}
