#include "scieppend/core/comparator.h"

int compare_int(const void* lhs, const void* rhs)
{
    return *(const int*)lhs - *(const int*)rhs;
}
