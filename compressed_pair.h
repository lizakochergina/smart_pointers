#pragma once

#include <type_traits>
#include <utility>

template <typename T, std::size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElement {
    T value_;

    CompressedPairElement() = default;
    template <typename U>
    CompressedPairElement(U&& value) noexcept : value_(std::forward<U>(value)) {}

    T& Get() {
        return value_;
    }
    const T& Get() const {
        return value_;
    }
};

template <typename T, std::size_t I>
struct CompressedPairElement<T, I, true> : public T {

    CompressedPairElement() = default;
    template <typename U>
    CompressedPairElement(U&&) noexcept {}

    T& Get() {
        return *this;
    }
    const T& Get() const {
        return *this;
    };
};

template <typename F, typename S>
class CompressedPair : private CompressedPairElement<F, 0>, private CompressedPairElement<S, 1> {
public:
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;

    CompressedPair() : First(), Second() {
    }
    CompressedPair(const F& first, const S& second) : First(first), Second(second) {
    }
    CompressedPair(F&& first, S&& second) : First(std::move(first)), Second(std::move(second)) {
    }
    CompressedPair(const F& first, S&& second) : First(first), Second(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : First(std::move(first)), Second(second) {
    }

    F& GetFirst() {
        return First::Get();
    }
    const F& GetFirst() const {
        return First::Get();
    }
    S& GetSecond() {
        return Second::Get();
    }
    const S& GetSecond() const {
        return Second::Get();
    }
};
