#include "vector.hpp"

#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>

template <typename T>
static inline void DestroyDiscard(T& object) {
    if constexpr (std::is_same_v<T, void*>) {
        if (object) {
            std::free(object);
        }
    }
    object.~T();
}

template <typename T>
Vector<T>::Vector() : data_(nullptr), size_(0), capacity_(0) {
}

template <typename T>
Vector<T>::Vector(size_t count, const T& value) : data_(nullptr), size_(0), capacity_(0) {
    if (count) {
        capacity_ = count;
        data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));
        for (size_t i = 0; i < count; ++i) {
            new (data_ + i) T(value);
        }
        size_ = count;
    }
}

template <typename T>
Vector<T>::Vector(const Vector& other) : data_(nullptr), size_(0), capacity_(0) {
    if (other.size_) {
        capacity_ = other.size_;
        data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));
        for (size_t i = 0; i < other.size_; ++i) {
            new (data_ + i) T(other.data_[i]);
        }
        size_ = other.size_;
    }
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& other) {
    if (this == &other) {
        return *this;
    }
    if (other.size_ > capacity_) {
        for (size_t i = 0; i < size_; ++i) {
            DestroyDiscard(data_[i]);
        }
        ::operator delete(data_);
        capacity_ = other.size_;
        data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));
        size_ = 0;
    } else {
        for (size_t i = 0; i < size_; ++i) {
            DestroyDiscard(data_[i]);
        }
        size_ = 0;
    }
    for (size_t i = 0; i < other.size_; ++i) {
        new (data_ + i) T(other.data_[i]);
    }
    size_ = other.size_;
    return *this;
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& other) {
    if (this == &other) {
        return *this;
    }
    for (size_t i = 0; i < size_; ++i) {
        DestroyDiscard(data_[i]);
    }
    ::operator delete(data_);
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
    return *this;
}

template <typename T>
Vector<T>::Vector(std::initializer_list<T> init) : data_(nullptr), size_(0), capacity_(0) {
    if (init.size()) {
        capacity_ = init.size() < kInitialCapacity ? kInitialCapacity : init.size();
        data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));
        size_t index = 0;
        for (auto& element : init) {
            new (data_ + index++) T(element);
        }
        size_ = init.size();
    }
}

template <typename T>
T& Vector<T>::operator[](size_t position) {
    return data_[position];
}

template <typename T>
T& Vector<T>::Front() const noexcept {
    return data_[0];
}

template <typename T>
T& Vector<T>::Back() const noexcept {
    return data_[size_ - 1];
}

template <typename T>
T* Vector<T>::Data() const noexcept {
    return data_;
}

template <typename T>
bool Vector<T>::IsEmpty() const noexcept {
    return size_ == 0;
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
void Vector<T>::Reserve(size_t new_capacity) {
    if (new_capacity == 0 || new_capacity <= capacity_) {
        return;
    }
    T* new_data = static_cast<T*>(::operator new(sizeof(T) * new_capacity));
    for (size_t i = 0; i < size_; ++i) {
        new (new_data + i) T(std::move(data_[i]));
        data_[i].~T();
    }
    ::operator delete(data_);
    data_ = new_data;
    capacity_ = new_capacity;
}

template <typename T>
void Vector<T>::Clear() noexcept {
    for (size_t i = 0; i < size_; ++i) {
        DestroyDiscard(data_[i]);
    }
    size_ = 0;
}

template <typename T>
void Vector<T>::Insert(size_t position, T value) {
    if (position > size_) {
        return;
    }
    if (capacity_ == 0) {
        Reserve(kInitialCapacity);
    }
    if (size_ == capacity_) {
        Reserve(capacity_ * 2);
    }
    for (size_t i = size_; i > position; --i) {
        new (data_ + i) T(std::move(data_[i - 1]));
        data_[i - 1].~T();
    }
    new (data_ + position) T(std::move(value));
    ++size_;
}

template <typename T>
void Vector<T>::Erase(size_t begin_position, size_t end_position) {
    if (begin_position >= size_ || end_position > size_ || begin_position >= end_position) {
        return;
    }
    size_t elements_to_remove = end_position - begin_position;
    for (size_t i = begin_position; i < end_position; ++i) {
        DestroyDiscard(data_[i]);
    }
    for (size_t i = end_position; i < size_; ++i) {
        new (data_ + i - elements_to_remove) T(std::move(data_[i]));
        data_[i].~T();
    }
    size_ -= elements_to_remove;
}

template <typename T>
void Vector<T>::PushBack(T value) {
    if (capacity_ == 0) {
        Reserve(kInitialCapacity);
    } else if (size_ == capacity_) {
        Reserve(capacity_ * 2);
    }
    new (data_ + size_) T(std::move(value));
    ++size_;
}

template <typename T>
template <class... Args>
void Vector<T>::EmplaceBack(Args&&... args) {
    if (capacity_ == 0) {
        Reserve(kInitialCapacity);
    } else if (size_ == capacity_) {
        Reserve(capacity_ * 2);
    }
    new (data_ + size_) T(std::forward<Args>(args)...);
    ++size_;
}

template <typename T>
void Vector<T>::PopBack() {
    if (size_ == 0) {
        return;
    }
    DestroyDiscard(data_[size_ - 1]);
    --size_;
}

template <typename T>
void Vector<T>::Resize(size_t count, const T& value) {
    if (count < size_) {
        for (size_t i = count; i < size_; ++i) {
            DestroyDiscard(data_[i]);
        }
        size_ = count;
        return;
    }
    if (count > capacity_) {
        size_t new_capacity = capacity_ ? capacity_ : kInitialCapacity;
        while (new_capacity < count) {
            new_capacity *= 2;
        }
        Reserve(new_capacity);
    }
    for (size_t i = size_; i < count; ++i) {
        new (data_ + i) T(value);
    }
    size_ = count;
}

template <typename T>
Vector<T>::~Vector() {
    for (size_t i = 0; i < size_; ++i) {
        DestroyDiscard(data_[i]);
    }
    ::operator delete(data_);
}
