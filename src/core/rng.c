#include "scieppend/core/rng.h"

#include <limits.h>
#include <stdlib.h>

// CONSTS

#define C_N 624

const int C_M = 397;
const int C_A = 0x9908b0df;
const int C_U = 11;
const int C_D = 0xffffffff;
const int C_S = 7;
const int C_B = 0x9d2c5680;
const int C_T = 15;
const int C_C = 0xefc60000;
const int C_L = 18;
const int C_F = 0x9b73cf98;
const unsigned int C_W = 32;
const unsigned int C_R = C_W - 1;
const unsigned int C_LOWER_MASK = (1LL << C_R) - 1;
const unsigned int C_UPPER_MASK = ~C_LOWER_MASK;

// STRUCTS

struct RNG
{
    int mt[C_N];
    int index;
};

// INTERNAL FUNCS

static void _twist(struct RNG* rng)
{
    for(int i = 0; i < C_N; ++i)
    {
        int x = (rng->mt[i] & C_UPPER_MASK) + (rng->mt[(i+1) % C_N] & C_LOWER_MASK);
        int xA = x >> 1;

        if((x % 2) != 0)
        {
            xA ^= C_A;
        }

        rng->mt[i] = rng->mt[(i + C_M ) % C_N] ^ xA;
    }

    rng->index = 0;
}

// EXTERNAL FUNCS

struct RNG* rng_new(int seed)
{
    struct RNG* rng = malloc(sizeof(struct RNG));

    rng->index = C_N;
    rng->mt[0] = seed;

    for(int i = 1; i < C_N; ++i)
    {
        int test = (C_F * (rng->mt[i-1] ^ (rng->mt[i-1] >> (C_W - 2))) + i);
        rng->mt[i] = C_LOWER_MASK & test;
    }

    return rng;
}

void rng_free(struct RNG* rng)
{
    free(rng);
}

int rng_get(struct RNG* rng)
{
    if(rng->index >= C_N)
    {
        _twist(rng);
    }

    int value = rng->mt[rng->index];
    value ^= ((value >> C_U) & C_D);
    value ^= ((value << C_S) & C_B);
    value ^= ((value << C_T) & C_C);
    value ^= (value >> C_L);

    ++rng->index;

    return C_LOWER_MASK & value;
}

float rng_get_float(struct RNG* rng)
{
    return (float)rng_get(rng) / (float)INT_MAX;
}
