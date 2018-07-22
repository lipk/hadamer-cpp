#pragma once
#include <cstdint>

typedef uint64_t u64;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint8_t u8;
typedef int8_t i8;

typedef u64 id_t;

#define NOT_COPYABLE(Name)                                                     \
    Name(const Name&) = delete;                                                \
    Name& operator=(const Name&) = delete;

#define NOT_MOVABLE(Name)                                                      \
    Name(Name&&) = delete;                                                     \
    Name& operator=(Name&&) = delete;
