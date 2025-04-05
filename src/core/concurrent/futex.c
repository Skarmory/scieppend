#include "scieppend/core/concurrent/futex.h"

#define _GNU_SOURCE

#include <linux/futex.h>
#include <sys/syscall.h>
#include <stddef.h>
#include <unistd.h>

int futex_wait(atomic_int* address, int expected)
{
    return syscall(SYS_futex, address, FUTEX_WAIT, expected, NULL, NULL, 0);
}

int futex_wake(atomic_int* address, int count)
{
    return syscall(SYS_futex, address, FUTEX_WAKE, count, NULL, NULL, 0);
}
