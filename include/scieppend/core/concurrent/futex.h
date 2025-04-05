#ifndef SCIEPPEND_CORE_CONCURRENT_FUTEX_H
#define SCIEPPEND_CORE_CONCURRENT_FUTEX_H

#include <stdatomic.h>

int futex_wait(atomic_int* address, int expected);
int futex_wake(atomic_int* address, int count);

#endif

