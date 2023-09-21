#include "scieppend/core/iterator.h"

bool it_eq(const struct It* lhs, const struct It* rhs)
{
    return lhs->container == rhs->container &&
           lhs->index == rhs->index;
}
