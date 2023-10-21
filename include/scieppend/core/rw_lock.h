#ifndef SCIEPPEND_CORE_RW_LOCK_H
#define SCIEPPEND_CORE_RW_LOCK_H

#include <threads.h>

struct RWLock
{
    mtx_t lock;
    cnd_t signal_r;
    cnd_t signal_w;

    int readers;
    int writers;
    int readers_waiting;
    int writers_waiting;
};

void rwlock_init(struct RWLock* lock);
void rwlock_uninit(struct RWLock* lock);
void rwlock_read_lock(struct RWLock* lock);
void rwlock_read_unlock(struct RWLock* lock);
void rwlock_write_lock(struct RWLock* lock);
void rwlock_write_unlock(struct RWLock* lock);

#endif

