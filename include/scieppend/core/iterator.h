#ifndef SCIEPPEND_CORE_ITERATOR_H
#define SCIEPPEND_CORE_ITERATOR_H

#include <stdbool.h>

struct It;

typedef struct It(*it_next_fn)(struct It it);
typedef bool(*it_eq_fn)(struct It lhs, struct It rhs);

struct It
{
    void*      container;
    int        index;
    it_next_fn next;
};

bool it_eq(const struct It* lhs, const struct It* rhs);

#endif

