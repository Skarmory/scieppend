#ifndef SCIEPPEND_CORE_RW_LOCK_H
#define SCIEPPEND_CORE_RW_LOCK_H

#include <threads.h>

extern const bool WRITE;
extern const bool READ;

struct RWLock
{
    mtx_t lock;
    cnd_t signal_r;
    cnd_t signal_w;

    int readers;
    int writers;
    int readers_waiting;
    int writers_waiting;
    bool kill;
};

void rwlock_init(struct RWLock* lock);
void rwlock_uninit(struct RWLock* lock);
bool rwlock_read_lock(struct RWLock* lock);
void rwlock_read_unlock(struct RWLock* lock);
bool rwlock_write_lock(struct RWLock* lock);
void rwlock_write_unlock(struct RWLock* lock);
bool rwlock_lock(struct RWLock* lock, bool locktype);
void rwlock_unlock(struct RWLock* lock, bool locktype);
void rwlock_set_kill(struct RWLock* lock);

void rwlock_init_wrapper(void* lock, const void* args);
void rwlock_uninit_wrapper(void* lock);

#endif

