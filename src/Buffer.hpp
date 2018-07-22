#pragma once

#include <array>
#include <cassert>

#include "Types.hpp"

template<class T, u8 dim>
struct Buffer
{
    NOT_COPYABLE(Buffer)
    NOT_MOVABLE(Buffer)

    const std::array<u64, dim> size;
    const std::array<u64, dim - 1> stride;
    const T* data;

    Buffer(std::array<u64, dim> size)
        : size(std::move(size))
    {
        u64 bufferSize = size[0];
        for (size_t i = 1; i < dim; ++i) {
            this->stride[i - 1] = bufferSize;
            bufferSize *= size[i];
        }
        this->data = new T[bufferSize];
    }
    ~Buffer() { delete[] this->data; }
};

template<class T, u8 dim>
struct Array
{
    NOT_COPYABLE(Array)
    NOT_MOVABLE(Array)

    const Buffer<T, dim>* buffer;
    const std::array<u64, dim> position, size;
    const u64 offset;

    Array(const Buffer<T, dim>* buffer,
          std::array<u64, dim> position,
          std::array<u64, dim> size)
        : buffer(buffer)
        , position(std::move(position))
        , size(std::move(size))
    {
        this->offset = this->position[0];
        for (u8 i = 0; i < dim - 1; ++i) {
            this->offset += this->size[i] * this->position[i + 1];
        }
    }

    inline const T& operator[](const std::array<u64, dim>& coords) const
    {
        u64 index = this->offset + coords[0];
        for (u8 i = 0; i < dim - 1; ++i) {
            index += coords[i + 1] * this->buffer->stride[i];
        }
        return this->buffer->data[index];
    }

    inline T& operator[](const std::array<u64, dim>& coords)
    {
        return const_cast<T&>(this->operator[](coords));
    }
};
