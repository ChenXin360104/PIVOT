#ifndef UTILS_H
#define UTILS_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <vector>

inline int BitScanReverse(size_t n) {
    // size_t 是无符号整数类型，位数取决于平台    
    return n ? (sizeof(size_t) * 8 - 1 - __builtin_clzl(n)) : -1;
}

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

typedef double PT;
typedef u64 UT;
typedef i64 ST;

extern u32 BLOCK_SIZE;
extern u32 MAX_THREADS;

u32 ceil(const u32& a, const u32& b);
#endif //UTILS_H