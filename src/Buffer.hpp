#pragma once

#include <array>
#include <cassert>
#include <memory>

#include <Types.hpp>
#include <Util.hpp>

template<class T, u8 dim>
struct Buffer
{
    static_assert (dim > 0, "Dimension must not be 0");
    NOT_COPYABLE(Buffer)
    NOT_MOVABLE(Buffer)

    using DataType = T;

    std::array<u64, dim> size;
    std::array<u64, dim - 1> stride;
    T* data;

    Buffer(std::array<u64, dim> size);
    ~Buffer() { delete[] this->data; }
};

template<typename T, u8 dim>
struct Array
{
    using DataType = T;

    std::shared_ptr<Buffer<T, dim>> buffer;
    std::array<u64, dim> position, size;
    u64 offset;

    Array(std::shared_ptr<Buffer<T, dim>> buffer,
          u64vec<dim> position,
          u64vec<dim> size);

    Array(const Array<T, dim>&) = default;
    Array(Array<T, dim>&&) = default;
    Array<T, dim>& operator =(const Array<T, dim>&) = default;
    Array<T, dim>& operator =(Array<T, dim>&&) = default;

    static Array<T, dim> createWithBuffer(u64vec<dim> size);
    inline const T& operator[](const u64vec<dim>& coords) const;
    inline T& operator[](const u64vec<dim>& coords);

    struct Getter
    {
        Array<T, dim> &array;
        const u64vec<dim> base;

        Getter(Array<T, dim>& array, const u64vec<dim>& base)
            : array(array)
            , base(base)
        {}

        template<typename ...XS>
        inline const T& operator()(i64 x1, XS... xs) const {
            auto offset = collect<dim>(x1, xs...);
            return array[base + offset];
        }

        template<typename ...XS>
        inline T& operator()(i64 x1, XS... xs) {
            auto offset = collect<dim>(x1, xs...);
            return array[base + offset];
        }
    };

    Getter getter(const u64vec<dim>& base) {
        return Getter(*this, base);
    }

    const Getter getter(const u64vec<dim>& base) const {
        return Getter(*this, base);
    }
};

#include <Buffer.impl.hpp>
