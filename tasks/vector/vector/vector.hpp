#pragma once
#include <cstddef>
#include <initializer_list>
#include <utility>

template <typename T>
class Vector {
public:
    Vector();
    Vector(size_t count, const T& value);
    Vector(const Vector& other);
    Vector& operator=(const Vector& other);
    Vector(Vector&& other) noexcept;
    Vector& operator=(Vector&& other);
    Vector(std::initializer_list<T> init);

    T& operator[](size_t position);

    T& Front() const noexcept;
    T& Back() const noexcept;
    T* Data() const noexcept;

    bool IsEmpty() const noexcept;
    size_t Size() const noexcept;
    size_t Capacity() const noexcept;

    void Reserve(size_t new_capacity);
    void Clear() noexcept;

    void Insert(size_t position, T value);
    void Erase(size_t begin_position, size_t end_position);

    void PushBack(T value);

    template <class... Args>
    void EmplaceBack(Args&&... args);

    void PopBack();
    void Resize(size_t count, const T& value);

    ~Vector();

private:
    static constexpr size_t InitialCapacity = 10;  // Changed from kInitialCapacity to InitialCapacity
    mutable T* data_;
    size_t size_;
    size_t capacity_;
};
