#pragma once

#include <array>
#include <exception>
#include <cassert>

#include <Types.hpp>

template<u64 len>
using u64vec = std::array<u64, len>;
template<u64 len>
using i64vec = std::array<i64, len>;

template<u64 len>
u64vec<len> operator +(const u64vec<len>& lhs, const u64vec<len>& rhs);
template<u64 len>
u64vec<len> operator +(const i64vec<len>& lhs, const u64vec<len>& rhs);
template<u64 len>
u64vec<len> operator +(const u64vec<len>& lhs, const i64vec<len>& rhs);
template<u64 len>
u64vec<len> operator -(const u64vec<len>& lhs, const u64vec<len>& rhs);
template<u64 len>
u64vec<len> operator -(const i64vec<len>& lhs, const u64vec<len>& rhs);
template<u64 len>
u64vec<len> operator -(const u64vec<len>& lhs, const i64vec<len>& rhs);
template<u64 len>
u64vec<len> operator *(const u64vec<len>& lhs, u64 rhs);
template<u64 len>
u64vec<len> operator *(u64 lhs, const u64vec<len>& rhs);
template<u64 len>
i64vec<len> operator %(const i64vec<len>& lhs, int rhs);

template <u8 L, typename T>
std::array<T, L> repeat(T t);

template<u8 dim, typename T, typename ...TS>
std::array<T, dim> collect(T t, TS... ts);

#define TOSTRING2(x) #x
#define TOSTRING(x) TOSTRING2(x)

#define CHECK_ARG(expr)                                                   \
    if (!(expr)) {                                                        \
        throw std::invalid_argument("Invalid argument: " #expr            \
            " evaluated to false at " __FILE__ ":" TOSTRING(__LINE__));   \
    }

#define ASSERT(expr) \
    assert(expr)

constexpr u64 ipow(u64 base, u64 exp)
{
    return exp == 0 ? 1 : base * ipow(base, exp-1);
}

#define BRANCH_FACTOR(dim) ipow(2, (dim))
#define NEIGHBOURHOOD_SIZE(dim) ipow(3, (dim))
