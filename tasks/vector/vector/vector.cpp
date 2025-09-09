#include "vector.hpp"

#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>

template <typename T>
static inline void DestroyDiscard(T& object) {
    object.~T();
}

template <typename T>
Vector<T>::Vector() : data_(nullptr), size_(0), capacity_(0) {
}

template <typename T>
Vector<T>::Vector(size_t count, const T& value) : data_(nullptr), size_(0), capacity_(0) {
    if (count > 0) {
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
    if (other.size_ > 0) {
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
    
    // Create temporary copy to handle self-assignment and exception safety
    Vector temp(other);
    
    // Swap with temporary
    std::swap(data_, temp.data_);
    std::swap(size_, temp.size_);
    std::swap(capacity_, temp.capacity_);
    
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
    
    // Clean up current resources
    for (size_t i = 0; i < size_; ++i) {
        data_[i].~T();
    }
    ::operator delete(data_);
    
    // Take ownership of other's resources
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    
    // Leave other in valid state
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
    
    return *this;
}

template <typename T>
Vector<T>::Vector(std::initializer_list<T> init) : data_(nullptr), size_(0), capacity_(0) {
    if (init.size() > 0) {
        capacity_ = init.size() < InitialCapacity ? InitialCapacity : init.size();
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
    if (new_capacity <= capacity_) {
        return;
    }
    
    T* new_data = static_cast<T*>(::operator new(sizeof(T) * new_capacity));
    
    // Move existing elements to new storage
    for (size_t i = 0; i < size_; ++i) {
        try {
            new (new_data + i) T(std::move(data_[i]));
        } catch (...) {
            // If construction fails, destroy what we've built so far and clean up
            for (size_t j = 0; j < i; ++j) {
                new_data[j].~T();
            }
            ::operator delete(new_data);
            throw;
        }
        data_[i].~T();
    }
    
    ::operator delete(data_);
    data_ = new_data;
    capacity_ = new_capacity;
}

template <typename T>
void Vector<T>::Clear() noexcept {
    for (size_t i = 0; i < size_; ++i) {
        data_[i].~T();
    }
    size_ = 0;
}

template <typename T>
void Vector<T>::Insert(size_t position, T value) {
    if (position > size_) {
        return;
    }
    
    if (size_ == capacity_) {
        size_t new_capacity = capacity_ == 0 ? InitialCapacity : capacity_ * 2;
        Reserve(new_capacity);
    }
    
    // Move elements to make space
    if (position < size_) {
        new (data_ + size_) T(std::move(data_[size_ - 1]));
        for (size_t i = size_ - 1; i > position; --i) {
            data_[i] = std::move(data_[i - 1]);
        }
        data_[position].~T();
    }
    
    // Construct new element
    new (data_ + position) T(std::move(value));
    ++size_;
}

template <typename T>
void Vector<T>::Erase(size_t begin_position, size_t end_position) {
    if (begin_position >= size_ || end_position > size_ || begin_position >= end_position) {
        return;
    }
    
    size_t elements_to_remove = end_position - begin_position;
    
    // Destroy elements in the range to be removed
    for (size_t i = begin_position; i < end_position; ++i) {
        data_[i].~T();
    }
    
    // Move elements after the range forward
    for (size_t i = end_position; i < size_; ++i) {
        new (data_ + i - elements_to_remove) T(std::move(data
