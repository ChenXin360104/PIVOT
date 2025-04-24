#include "utils.h"
#include "alp.hpp"
#include <iostream>

u32 BLOCK_SIZE = alp::config::VECTOR_SIZE;
u32 MAX_THREADS;

u32 ceil(const u32& a, const u32& b)
{
    return a ? (a - 1) / b + 1 : 0;
}