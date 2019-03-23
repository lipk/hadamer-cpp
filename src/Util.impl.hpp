#pragma once
#include <Util.hpp>
#include <initializer_list>
#include <array>

template<u64 len>
u64vec<len> operator +(const u64vec<len>& lhs, const u64vec<len>& rhs)
{
    u64vec<len> result;
    for (u8 i = 0; i<len; ++i) {
        result[i] = lhs[i] + rhs[i];
    }
    return result;
}

template<u64 len>
u64vec<len> operator +(const i64vec<len>& lhs, const u64vec<len>& rhs)
{
    u64vec<len> result;
    for (u8 i = 0; i<len; ++i) {
        result[i] = lhs[i] + rhs[i];
    }
    return result;
}

template<u64 len>
u64vec<len> operator +(const u64vec<len>& lhs, const i64vec<len>& rhs)
{
    return rhs + lhs;
}

template<u64 len>
u64vec<len> operator -(const u64vec<len>& lhs, const u64vec<len>& rhs)
{
    u64vec<len> result;
    for (u8 i = 0; i<len; ++i) {
        result[i] = lhs[i] - rhs[i];
    }
    return result;
}

template<u64 len>
u64vec<len> operator -(const i64vec<len>& lhs, const u64vec<len>& rhs)
{
    u64vec<len> result;
    for (u8 i = 0; i<len; ++i) {
        result[i] = lhs[i] - rhs[i];
    }
    return result;
}

template<u64 len>
u64vec<len> operator -(const u64vec<len>& lhs, const i64vec<len>& rhs)
{
    return rhs - lhs;
}

template<u64 len>
u64vec<len> operator *(const u64vec<len>& lhs, u64 rhs)
{
    u64vec<len> result;
    for (u8 i = 0; i<len; ++i) {
        result[i] = rhs * lhs[i];
    }
    return result;
}

template<u64 len>
u64vec<len> operator *(u64 lhs, const u64vec<len>& rhs)
{
    return rhs * lhs;
}

template<u64 len>
u64vec<len> operator %(const u64vec<len>& lhs, int rhs)
{
    u64vec<len> result;
    for (u8 i = 0; i<len; ++i) {
        result[i] = lhs[i] % rhs;
    }
    return result;
}

template <u8 L, typename T>
std::array<T, L> repeat(T t) {
    std::array<T, L> xs;
    for (auto& x : xs) {
        x = t;
    }
    return xs;
}

template<u8 dim, u8 index, typename A>
void collectImpl(std::array<A, dim>&)
{
}

template<u8 dim, u8 index, typename A, typename T, typename ...TS>
void collectImpl(std::array<A, dim>& arr, T t, TS... ts)
{
    arr[index] = t;
    collectImpl<dim, index+1, A, TS...>(arr, ts...);
}

template<u8 dim, typename T, typename ...TS>
std::array<T, dim> collect(T t, TS... ts)
{
    std::array<T, dim> arr;
    collectImpl<dim, 0, T, T, TS...>(arr, t, ts...);
    return arr;
}
