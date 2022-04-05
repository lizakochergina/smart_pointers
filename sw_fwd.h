#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class ControlBlockBase {
public:
    size_t sp_cnt_ref_ = 0;
    size_t wp_cnt_ref_ = 0;
    virtual void DeleteObj(){};
    virtual ~ControlBlockBase(){};
};

template <typename T>
class ControlBlockPointer : public ControlBlockBase {
public:
    template <typename U>
    ControlBlockPointer(U* ptr) : ptr_(ptr) {
    }
    ~ControlBlockPointer() {
    }
    void DeleteObj() override {
        if (ptr_) {
            delete ptr_;
        }
    }

private:
    T* ptr_;
};

template <typename T>
class ControlBlockHolder : public ControlBlockBase {
public:
    template <typename... Args>
    ControlBlockHolder(Args&&... args) {
        new (&storage_) T(std::forward<Args>(args)...);
    }
    T* GetPtr() {
        return reinterpret_cast<T*>(&storage_);
    }

    ~ControlBlockHolder() {
    }

    void DeleteObj() override {
        reinterpret_cast<T*>(&storage_)->~T();
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};