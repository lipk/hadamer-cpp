#pragma once
#include <cstdint>
#include <exception>

typedef uint64_t u64;
typedef int64_t i64;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint8_t u8;
typedef int8_t i8;


#define NOT_COPYABLE(Name)                                                     \
    Name(const Name&) = delete;                                                \
    Name& operator=(const Name&) = delete;

#define NOT_MOVABLE(Name)                                                      \
    Name(Name&&) = delete;                                                     \
    Name& operator=(Name&&) = delete;
