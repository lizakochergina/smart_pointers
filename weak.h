#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    // Constructors

    WeakPtr() noexcept : ptr_(nullptr), block_(nullptr) {
    }
    WeakPtr(const WeakPtr& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->wp_cnt_ref_;
        }
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->wp_cnt_ref_;
        }
    }

    template <typename U>
    WeakPtr(const SharedPtr<U>& other) noexcept : ptr_(other.Get()), block_(other.GetBlock()) {
        if (block_) {
            ++block_->wp_cnt_ref_;
        }
    }

    WeakPtr(WeakPtr&& other) noexcept
        : ptr_(std::move(other.ptr_)), block_(std::move(other.block_)) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    template <typename U>
    WeakPtr(WeakPtr<U>&& other) noexcept
        : ptr_(std::move(other.ptr_)), block_(std::move(other.block_)) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) noexcept {
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(const WeakPtr<U>& other) noexcept {
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(const SharedPtr<U>& other) noexcept {
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept {
        WeakPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(WeakPtr<U>&& other) noexcept {
        WeakPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    // Destructor

    ~WeakPtr() {
        if (block_) {
            if (block_->sp_cnt_ref_ > 0) {
                --block_->wp_cnt_ref_;
            } else {
                if (block_->wp_cnt_ref_ == 1) {
                    delete block_;
                } else {
                    --block_->wp_cnt_ref_;
                }
            }
        }
    }

    // Modifiers

    void Reset() {
        if (block_) {
            --block_->wp_cnt_ref_;
        }
        ptr_ = nullptr;
        block_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    // Observers

    size_t UseCount() const {
        if (block_) {
            return block_->sp_cnt_ref_;
        } else {
            return 0;
        }
    }
    bool Expired() const {
        return UseCount() == 0;
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        } else {
            return SharedPtr<T>(*this);
        }
    }

private:
    T* ptr_;
    ControlBlockBase* block_;

    template <typename U>
    friend class WeakPtr;

    template <typename U>
    friend class SharedPtr;

    friend class SharedPtr<T>;
};
