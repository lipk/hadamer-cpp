#include <Buffer.hpp>

template<class T, u8 dim>
Buffer<T, dim>::Buffer(std::array<u64, dim> size)
    : size(std::move(size))
{
    u64 bufferSize = size[0];
    CHECK_ARG(size[0] != 0);
    for (size_t i = 1; i < dim; ++i) {
        CHECK_ARG(size[i] != 0);
        this->stride[i - 1] = bufferSize;
        bufferSize *= size[i];
    }
    this->data = new T[bufferSize];
}

template<class T, u8 dim>
Array<T, dim>::Array(std::shared_ptr<Buffer<T, dim> > buffer,
                     u64vec<dim> position, u64vec<dim> size)
    : buffer(buffer)
    , position(std::move(position))
    , size(std::move(size))
{
    this->offset = this->position[0];
    for (u8 i = 0; i < dim - 1; ++i) {
        this->offset += this->size[i] * this->position[i + 1];
    }
}

template<class T, u8 dim>
Array<T, dim> Array<T, dim>::createWithBuffer(std::array<u64, dim> size)
{
    auto buffer = std::make_shared<Buffer<T, dim>>(size);
    return Array(std::move(buffer), repeat<dim, u64>(0), size);
}

template<class T, u8 dim>
const T &Array<T, dim>::operator[](const std::array<u64, dim> &coords) const
{
    u64 index = this->offset + coords[0];
    for (u8 i = 0; i < dim - 1; ++i) {
        index += coords[i + 1] * this->buffer->stride[i];
    }
    return this->buffer->data[index];
}

template<class T, u8 dim>
T &Array<T, dim>::operator[](const std::array<u64, dim> &coords)
{
    return const_cast<T&>(const_cast<const Array<T, dim>*>(this)->operator[](coords));
}
