#pragma once

/// GCD - Greatest common divisor with two arguments
template <typename T>
constexpr T gcd(const T &x, const T &y) noexcept {
    if (y == 0) {
        return x;
    }
    return gcd(y, static_cast<T>(x % y));
}
static_assert(gcd(2, 4) == 2, "Error");
static_assert(gcd(3, 15) == 3, "Error");
static_assert(gcd(12, 15) == 3, "Error");
static_assert(gcd(1071, 462) == 21, "Error");

/// GCD - Greatest common divisor with more than two arguments
template <typename T, typename... Args>
constexpr T gcd(const T &x, const T &y, Args... args) {
    return gcd(gcd(x, y), args...);
}

/// Sort two numbers without using temporary (triple XOR)
template <typename T>
inline constexpr void sort2(T &a, T &b) noexcept {
    if (a > b) { // fancy swap, saves the memory for one temporary
        a ^= b;
        b ^= a;
        a ^= b;
    }
}

/// Sort three numbers without using temporary (triple XOR * 3)
template <typename T>
inline constexpr void sort3(T &a, T &b, T &c) noexcept {
    sort2(a, b);
    sort2(b, c);
    sort2(a, b);
}
