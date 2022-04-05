#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <iostream>


// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    // Constructors

    SharedPtr() noexcept : ptr_(nullptr), block_(nullptr) {
    }
    SharedPtr(std::nullptr_t) : ptr_(nullptr), block_(nullptr) {
    }

    template <typename U>
    explicit SharedPtr(U* ptr) noexcept : ptr_(ptr), block_(new ControlBlockPointer<U>(ptr)) {
        ++block_->sp_cnt_ref_;
    }

    SharedPtr(const SharedPtr& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->sp_cnt_ref_;
        }
    }
    template <typename U>
    SharedPtr(const SharedPtr<U>& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->sp_cnt_ref_;
        }
    }
    SharedPtr(SharedPtr&& other) noexcept
        : ptr_(std::move(other.ptr_)), block_(std::move(other.block_)) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }
    template <typename U>
    SharedPtr(SharedPtr<U>&& other) noexcept
        : ptr_(std::move(other.ptr_)), block_(std::move(other.block_)) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : ptr_(ptr), block_(other.block_) {
        if (block_) {
            ++block_->sp_cnt_ref_;
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.block_ && other.block_->sp_cnt_ref_ == 0) {
            throw BadWeakPtr{};
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_) {
            ++block_->sp_cnt_ref_;
        }
    }

    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        SharedPtr(other).Swap(*this);
        return *this;
    }
    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        SharedPtr<T>(other).Swap(*this);
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) {
        SharedPtr(std::move(other)).Swap(*this);
        return *this;
    }
    template <typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        SharedPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    // Destructor

    ~SharedPtr() {
        if (block_) {
            if (block_->sp_cnt_ref_ == 1) {
                if (block_->wp_cnt_ref_ == 0) {
                    block_->DeleteObj();
                    delete block_;
                } else {
                    --block_->sp_cnt_ref_;
                    block_->DeleteObj();
                }
            } else {
                --block_->sp_cnt_ref_;
            }
        }
    }

    // Modifiers

    void Reset() {
        SharedPtr().Swap(*this);
    }
    template <typename U>
    void Reset(U* ptr) {
        SharedPtr<T>(ptr).Swap(*this);
    }
    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (block_) {
            return block_->sp_cnt_ref_;
        } else {
            return 0;
        }
    }
    explicit operator bool() const {
        if (ptr_) {
            return true;
        } else {
            return false;
        }
    }
    ControlBlockBase* GetBlock() const {
        return block_;
    }

private:
    T* ptr_;
    ControlBlockBase* block_;

    template <typename U>
    friend class SharedPtr;

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new ControlBlockHolder<T>(std::forward<Args>(args)...);
    SharedPtr<T> sp;
    sp.block_ = block;
    sp.ptr_ = block->GetPtr();
    if (sp.ptr_ && sp.block_) {
        ++sp.block_->sp_cnt_ref_;
    }
    return sp;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
