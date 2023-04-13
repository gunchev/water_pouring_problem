#pragma once

/// GCD - Greatest common divisor with two arguments
template <typename T>
constexpr T gcd(const T &lhs, const T &rhs) noexcept {
    if (rhs == 0) {
        return lhs;
    }
    return gcd(rhs, static_cast<T>(lhs % rhs));
}
static_assert(gcd(2, 4) == 2, "Error at gcd(2, 4)");
static_assert(gcd(3, 15) == 3, "Error at gcd(3, 15)");
static_assert(gcd(12, 15) == 3, "Error at gcd(12, 15)");
static_assert(gcd(1071, 462) == 21, "Error at gcd(1071, 462)");

/// GCD - Greatest common divisor with more than two arguments
template <typename T, typename... Args>
constexpr T gcd(const T &lhs, const T &rhs, Args... args) {
    return gcd(gcd(lhs, rhs), args...);
}
static_assert(gcd(1071, 462, 84) == 21, "Error at gcd(1071, 462, 84)");
