#pragma once

#include "compressed_pair.h"
#include <iostream>
#include <cstddef>  // std::nullptr_t
#include <type_traits>

template <typename U>
struct DefaultDeleter {
    void operator()(U* ptr) {
        if (ptr) {
            delete ptr;
        }
    }
};

template <typename U>
struct DefaultDeleter<U[]> {
    void operator()(U* ptr) {
        if (ptr) {
            delete[] ptr;
        }
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr : private CompressedPairElement<Deleter, 0> {
public:
    using CompressedDeleter = CompressedPairElement<Deleter, 0>;

    // Constructors
    UniquePtr() noexcept : data_(nullptr), CompressedDeleter() {
    }
    explicit UniquePtr(T* ptr) noexcept : data_(ptr), CompressedDeleter() {
    }
    UniquePtr(UniquePtr&& other) noexcept
        : data_(other.Release()), CompressedDeleter(std::forward<Deleter>(other.GetDeleter())) {
    }
    template <typename U, typename D = DefaultDeleter<U>>
    UniquePtr(UniquePtr<U, D>&& other) noexcept
        : data_(other.Release()), CompressedDeleter(std::forward<D>(other.GetDeleter())) {
    }
    template <typename D>
    UniquePtr(T* ptr, D&& deleter) noexcept
        : data_(ptr), CompressedDeleter(std::forward<D>(deleter)) {
    }

    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        CompressedDeleter::Get() = std::forward<Deleter>(other.GetDeleter());
        this->Reset(other.Release());
        return *this;
    }
    template <typename U, typename D = DefaultDeleter<U>>
    UniquePtr& operator=(UniquePtr<U, D>&& other) noexcept {
        CompressedDeleter::Get() = std::forward<D>(other.GetDeleter());
        this->Reset(other.Release());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        delete data_;
        data_ = nullptr;
        CompressedDeleter::Get() = CompressedDeleter();
        return *this;
    }

    // Destructor

    ~UniquePtr() noexcept {
        CompressedDeleter::Get()(data_);
    }

    // Modifiers

    T* Release() noexcept {
        T* copy = nullptr;
        std::swap(copy, data_);
        return copy;
    }
    void Reset(T* ptr = nullptr) noexcept {
        auto old_prt = data_;
        data_ = ptr;
        if (old_prt) {
            delete old_prt;
        }
    }
    void Swap(UniquePtr& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(this->GetDeleter(), other.GetDeleter());
    }

    // Observers

    T* Get() const {
        return data_;
    };
    Deleter& GetDeleter() noexcept {
        return CompressedDeleter::Get();
    }
    const Deleter& GetDeleter() const noexcept {
        return CompressedDeleter::Get();
    }
    explicit operator bool() const noexcept {
        return (data_ != nullptr);
    }

    // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const {
        return *data_;
    }
    T* operator->() const {
        return data_;
    }

private:
    T* data_ = nullptr;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> : private CompressedPairElement<Deleter, 0> {
public:
    using CompressedDeleter = CompressedPairElement<Deleter, 0>;

    UniquePtr() noexcept : data_(nullptr), CompressedDeleter() {
    }
    explicit UniquePtr(T* ptr) noexcept : data_(ptr), CompressedDeleter() {
    }
    UniquePtr(UniquePtr&& other) noexcept
        : data_(other.Release()), CompressedDeleter(std::move(other.GetDeleter())) {
    }
    template <typename U, typename D = DefaultDeleter<U>>
    UniquePtr(UniquePtr<U[], D>&& other) noexcept
        : data_(other.Release()), CompressedDeleter(std::forward<D>(other.GetDeleter())) {
    }
    template <typename D>
    UniquePtr(T* ptr, D&& deleter) noexcept
        : data_(ptr), CompressedDeleter(std::forward<D>(deleter)) {
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        CompressedDeleter::Get = std::forward<Deleter>(other.GetDeleter());
        this->Reset(other.Release());
        return *this;
    }
    template <typename U, typename D = DefaultDeleter<U>>
    UniquePtr& operator=(UniquePtr<U[], D>&& other) noexcept {
        CompressedDeleter::Get() = std::forward<D>(other.GetDeleter());
        this->Reset(other.Release());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        delete data_;
        data_ = nullptr;
        CompressedDeleter::Get() = CompressedDeleter();
        return *this;
    }

    ~UniquePtr() noexcept {
        CompressedDeleter::Get()(data_);
    }

    T* Release() noexcept {
        T* copy = nullptr;
        std::swap(copy, data_);
        return copy;
    }
    void Reset(T* ptr = nullptr) noexcept {
        auto old_prt = data_;
        data_ = ptr;
        if (old_prt) {
            delete[] old_prt;
        }
    }
    void Swap(UniquePtr& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(other.GetDeleter(), this->GetDeleter());
    }

    T* Get() const {
        return data_;
    };
    Deleter& GetDeleter() noexcept {
        return CompressedDeleter::Get();
    }
    const Deleter& GetDeleter() const noexcept {
        return CompressedDeleter::Get();
    }
    explicit operator bool() const noexcept {
        return (data_ != nullptr);
    }

    T& operator[](std::size_t i) {
        return data_[i];
    }

    const T& operator[](std::size_t i) const {
        return data_[i];
    }

private:
    T* data_;
};
