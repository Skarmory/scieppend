#include "scieppend/core/noise.h"

#include "scieppend/core/log.h"
#include <math.h>

// CONSTS

static const float C_ROOT_2       = 1.41421356237f;
static const float C_PERLIN_MAX   = C_ROOT_2 * 0.5f;
static const float C_PERLIN_MIN   = -C_PERLIN_MAX;
static const float C_PERLIN_RANGE = 1.0f / (C_PERLIN_MAX - C_PERLIN_MIN);
static const float C_Z            = 3.14159265f / (float) ~(~0u >> 1);
static const unsigned C_W         = 8 * sizeof(unsigned);
static const unsigned C_S         = C_W / 2;

// INTERNAL FUNCS

static void _random_gradient(int x, int y, float* out_x, float* out_y)
{
    unsigned a = x;
    unsigned b = y;

    a *= 3284157443;
    b ^= a << C_S | a >> (C_W - C_S);

    b *= 1911520717;
    a ^= b << C_S | b >> (C_W - C_S);

    a *= 2048419325;

    float random = a * C_Z;

    *out_x = sin(random);
    *out_y = cos(random);
}

static float _interpolate(float a0, float a1, float w)
{
    return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}

// Based on algorithm found at: https://en.wikipedia.org/wiki/Perlin_noise
static float _perlin(float x, float y)
{
    // Get the for corners of the square that (x, y) lies in
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Create pseudorandom gradient vectors from each corner
    float g_x0y0_x;
    float g_x0y0_y;
    _random_gradient(x0, y0, &g_x0y0_x, &g_x0y0_y);

    float g_x1y0_x;
    float g_x1y0_y;
    _random_gradient(x1, y0, &g_x1y0_x, &g_x1y0_y);

    float g_x0y1_x;
    float g_x0y1_y;
    _random_gradient(x0, y1, &g_x0y1_x, &g_x0y1_y);

    float g_x1y1_x;
    float g_x1y1_y;
    _random_gradient(x1, y1, &g_x1y1_x, &g_x1y1_y);

    // Create distance vectors from corners to (x, y)
    float d_x0 = x - (float)x0; // This also is the x coordinate relative to the grid square
    float d_x1 = x - (float)x1;
    float d_y0 = y - (float)y0; // This also is the y coordinate relative to the grid square
    float d_y1 = y - (float)y1;

    // Dot product the gradient and the distance
    float dotx0y0 = (d_x0 * g_x0y0_x) + (d_y0 * g_x0y0_y);
    float dotx1y0 = (d_x1 * g_x1y0_x) + (d_y0 * g_x1y0_y);
    float dotx0y1 = (d_x0 * g_x0y1_x) + (d_y1 * g_x0y1_y);
    float dotx1y1 = (d_x1 * g_x1y1_x) + (d_y1 * g_x1y1_y);

    // Interpolate between the values
    float x_interpolate_1 = _interpolate(dotx0y0, dotx1y0, d_x0);
    float x_interpolate_2 = _interpolate(dotx0y1, dotx1y1, d_x0);
    float y_interpolate   = _interpolate(x_interpolate_1, x_interpolate_2, d_y0);

    return y_interpolate;
}

// EXTERNAL FUNCS

float perlin(float x, float y, float amplitude, float frequency, int octaves)
{
    float ret = 0.0f;

    for(int i = 0; i < octaves; ++i)
    {
        ret += amplitude * _perlin(x * frequency, y * frequency);
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    ret = (ret - C_PERLIN_MIN) * C_PERLIN_RANGE;

    return ret;
}
