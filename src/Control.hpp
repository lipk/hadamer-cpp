#pragma once

#include <tuple>
#include <array>

#include <Types.hpp>
#include <Util.hpp>

#define _tm template
#define _tn typename

template<typename Func, typename ...Items>
auto mapTuple(const std::tuple<Items...>& items, const Func& func)
{
    return std::apply([&](auto ...x) {
        return std::make_tuple(func(x)...);
    }, items);
}

template<typename Func, typename ...Items>
void forEachTuple(const std::tuple<Items...>& items, const Func& func)
{
    mapTuple(items, [&](const auto& x) {
        func(x);
        return 42;
    });
}

template <typename T>
struct TypeCell
{
    using Type = T;
};

template <typename Type_, u64 index_>
struct TypeListItem
{
    using Type = Type_;
    static constexpr const u64 index = index_;
};

template <typename Func, u64 index, typename T, typename ...TS>
void forEachImpl(const Func& func) {
    func(TypeListItem<T, index>());
    forEachImpl<Func, index+1, TS...>(func);
}

template <typename Func, u64 index>
void forEachImpl(const Func&) {}

template <typename ...TS>
struct TypeList
{
    template<template <typename> typename W>
    using Wrap = TypeList<W<TS>...>;

    template<template <typename> typename M>
    using Map = TypeList<_tn M<TS>::Result...>;

    template<template <typename ...> typename U>
    using To = U<TS...>;

    template<typename Func>
    static void forEach(const Func& func) {
        forEachImpl<Func, 0, TS...>(func);
    }

    using ToTuple = To<std::tuple>;
};

#define TYPEFUNC_GETTER(member)       \
    template <typename T>             \
    struct Get##member {              \
        using Result = _tn T::member; \
    }

template<u8 n, u8 m>
struct LoopImpl {
    template<class Body>
    static void run(const u64vec<n>& from, const u64vec<n>& to, const Body& body, std::array<u64, n>& i) {
        for (i[n-m] = from[n-m]; i[n-m]<to[n-m]; ++i[n-m]) {
            LoopImpl<n, m-1>::run(from, to, body, i);
        }
    }
};

template<u8 n>
struct LoopImpl<n, 0> {
    template<class Body>
    static void run(const u64vec<n>&, const u64vec<n>&, const Body& body, std::array<u64, n>& i) {
        body(i);
    }
};

template<u8 n>
struct Loop {
    template<class Body>
    Loop(u64 from, u64 to, const Body& body) {
        std::array<u64, n> i;
        auto fromv = repeat<n, u64>(from);
        auto tov = repeat<n, u64>(to);
        LoopImpl<n, n>::run(fromv, tov, body, i);
    }
    template<class Body>
    Loop(const u64vec<n>& from, const u64vec<n>& to, const Body& body) {
        u64vec<n> i;
        LoopImpl<n, n>::run(from, to, body, i);
    }
};

