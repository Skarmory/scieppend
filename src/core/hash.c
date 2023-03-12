#include "scieppend/core/hash.h"

const long C_FNV_OFFSET_BASIS = 2166136261;
const int  C_FNV_PRIME = 16777619;

// Simple FNV-1 hash
// See: https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
int hash(char* buffer, int size_bytes)
{
    int h = C_FNV_OFFSET_BASIS;

    for(int i = 0; i < size_bytes; ++i)
    {
        h *= C_FNV_PRIME;
        h ^= buffer[i];
    }

    return h;
}
