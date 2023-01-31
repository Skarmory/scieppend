#include "core/math_utils.h"

#include <math.h>

int clamp(int val, int low, int high)
{
    if(val < low) return low;
    if(val > high) return high;
    return val;
}

float log_base(int x, int base)
{
    return log(x) / log(base);
}

float map_range(float value, float old_low, float old_high, float new_low, float new_high)
{
    float f = (value - old_low) / (old_high - old_low);

    return (f * (new_high - new_low)) + new_low;
}

void math_change_basis(const int x, const int y, const int old_ox, const int old_oy, const int new_ox, const int new_oy, int* x_out, int* y_out)
{
    *x_out = (x - old_ox) + new_ox;
    *y_out = (y - old_oy) + new_oy;
}
