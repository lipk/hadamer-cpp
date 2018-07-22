#pragma once

#include <array>
#include <cassert>
#include <iostream>

#include "Types.hpp"

template<u8 dim>
class Frac;

struct FracRView
{
    u32 nominator;
    u8 denominator;
    inline u32 denom() const { return 1U << static_cast<u32>(denominator); }
    inline u32 nom() const { return nominator; }
};
#define FRAC_LVIEW_OP_DECL(op)                                                 \
    template<u8 m>                                                             \
    inline FracRView operator op(FracLView<m> rhs);                            \
    inline FracRView operator op(FracRView rhs);

template<u8 dim>
struct FracLView
{
    Frac<dim>& object;
    u8 index;
    inline operator FracRView() const
    {
        return { object.nominators[index], object.denominator };
    }
    inline u32 denom() const { return object.denom(); }
    inline u32 nom() const { return object.nominators[index]; }
    FRAC_LVIEW_OP_DECL(+=);
    FRAC_LVIEW_OP_DECL(-=);
    FRAC_LVIEW_OP_DECL(=);
};

template<u8 dim>
class Frac
{
    template<u8 m>
    friend class Frac;
    template<u8 m>
    friend void align(FracLView<m>& lhs, FracRView& rhs);
    friend struct FracLView<dim>;
    std::array<u32, dim> nominators;
    u8 denominator;

    template<u8 i, typename... args_t>
    inline void init()
    {
        static_assert(i == dim, "incorrect number of arguments");
    }
    template<u8 i, typename... args_t>
    inline void init(u32 x, args_t... xs)
    {
        std::get<i>(nominators) = x;
        init<i + 1>(xs...);
    }

  public:
    typedef FracLView<dim> lview;
    typedef FracRView rview;

    template<typename... args_t>
    inline Frac(args_t... xs)
        : denominator(0)
    {
        init<0>(xs...);
    }
    inline Frac()
        : nominators()
        , denominator(0)
    {
        for (auto& coord : nominators) {
            coord = 0U;
        }
    }
    inline void focus(u8 level)
    {
        this->denominator += level;
        for (auto& coord : nominators) {
            coord <<= level;
        }
    }
    inline void defocus(u8 level)
    {
        this->denominator -= level;
        for (auto& coord : nominators) {
            coord >>= level;
        }
    }
    inline FracLView<dim> operator[](u8 i) { return { *this, i }; }
    inline FracRView operator[](u8 i) const
    {
        return { nominators[i], denominator };
    }
    inline Frac<1> unit() const
    {
        Frac<1> res;
        res.focus(denominator);
        res.nominators[0] = 1;
        return res;
    }
    inline u32 nom(u8 i) const { return nominators[i]; }
    inline u32 denom() const { return 1 << static_cast<u32>(denominator); }
};

template<u8 m>
inline void align(FracLView<m>& lhs, FracRView& rhs)
{
    if (rhs.denominator < lhs.object.denominator) {
        rhs.nominator <<= lhs.object.denominator - rhs.denominator;
        rhs.denominator = lhs.object.denominator;
    } else {
        lhs.object.focus(rhs.denominator - lhs.object.denominator);
    }
}

inline void align(FracRView& lhs, FracRView& rhs)
{
    if (lhs.denominator < rhs.denominator) {
        lhs.nominator <<= rhs.denominator - lhs.denominator;
        lhs.denominator = rhs.denominator;
    } else {
        rhs.nominator <<= lhs.denominator - rhs.denominator;
        rhs.denominator = lhs.denominator;
    }
}

#define FRAC_RVIEW_OP(op, rettype)                                             \
    rettype operator op(FracRView lhs, FracRView rhs);                         \
    template<u8 m, u8 n>                                                       \
    inline rettype operator op(FracLView<m> lhs, FracLView<n> rhs)             \
    {                                                                          \
        return static_cast<FracRView>(lhs) op static_cast<FracRView>(rhs);     \
    }                                                                          \
    template<u8 m>                                                             \
    inline rettype operator op(FracLView<m> lhs, FracRView rhs)                \
    {                                                                          \
        return static_cast<FracRView>(lhs) op rhs;                             \
    }                                                                          \
    template<u8 m>                                                             \
    inline rettype operator op(FracRView lhs, FracLView<m> rhs)                \
    {                                                                          \
        return lhs op static_cast<FracRView>(rhs);                             \
    }                                                                          \
    inline rettype operator op(FracRView lhs, FracRView rhs)

FRAC_RVIEW_OP(+, FracRView)
{
    align(lhs, rhs);
    lhs.nominator += rhs.nominator;
    return lhs;
}
FRAC_RVIEW_OP(-, FracRView)
{
    align(lhs, rhs);
    lhs.nominator -= rhs.nominator;
    return lhs;
}
FRAC_RVIEW_OP(<, bool)
{
    align(lhs, rhs);
    return lhs.nominator < rhs.nominator;
}
FRAC_RVIEW_OP(>, bool)
{
    align(lhs, rhs);
    return lhs.nominator > rhs.nominator;
}
FRAC_RVIEW_OP(==, bool)
{
    align(lhs, rhs);
    return lhs.nominator == rhs.nominator;
}
FRAC_RVIEW_OP(<=, bool)
{
    align(lhs, rhs);
    return lhs.nominator <= rhs.nominator;
}
FRAC_RVIEW_OP(>=, bool)
{
    align(lhs, rhs);
    return lhs.nominator >= rhs.nominator;
}
FRAC_RVIEW_OP(!=, bool)
{
    align(lhs, rhs);
    return lhs.nominator != rhs.nominator;
}

#define FRAC_RVIEW_SCALAR_OP(op)                                               \
    inline FracRView operator op(FracRView lhs, i32 rhs);                      \
    template<u8 n>                                                             \
    inline FracRView operator op(FracLView<n> lhs, i32 rhs)                    \
    {                                                                          \
        return static_cast<FracRView>(lhs) op rhs;                             \
    }                                                                          \
    inline FracRView operator op(FracRView lhs, i32 rhs)

FRAC_RVIEW_SCALAR_OP(*)
{
    lhs.nominator *= rhs;
    return lhs;
}
FRAC_RVIEW_SCALAR_OP(/)
{
    lhs.nominator /= rhs;
    return lhs;
}

#define FRAC_LVIEW_OP_IMPL(op)                                                 \
    template<u8 dim>                                                           \
    template<u8 m>                                                             \
    inline FracRView FracLView<dim>::operator op(FracLView<m> rhs)             \
    {                                                                          \
        return *this op static_cast<FracRView>(rhs);                           \
    }                                                                          \
    template<u8 m>                                                             \
    inline FracRView FracLView<m>::operator op(FracRView rhs)

FRAC_LVIEW_OP_IMPL(+=)
{
    align(*this, rhs);
    object.nominators[index] += rhs.nominator;
    return static_cast<FracRView>(*this);
}
FRAC_LVIEW_OP_IMPL(-=)
{
    align(*this, rhs);
    object.nominators[index] -= rhs.nominator;
    return static_cast<FracRView>(*this);
}
FRAC_LVIEW_OP_IMPL(=)
{
    align(*this, rhs);
    object.nominators[index] = rhs.nominator;
    return static_cast<FracRView>(*this);
}

#undef FRAC_LVIEW_OP_DECL
#undef FRAC_LVIEW_OP_IMPL
#undef FRAC_RVIEW_OP
#undef FRAC_RVIEW_SCALAR_OP
