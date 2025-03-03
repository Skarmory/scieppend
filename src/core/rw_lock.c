#include "scieppend/core/rw_lock.h"

const bool WRITE = true;
const bool READ = false;

void rwlock_init(struct RWLock* lock)
{
    mtx_init(&lock->lock, mtx_plain);
    cnd_init(&lock->signal_r);
    cnd_init(&lock->signal_w);

    lock->readers = 0;
    lock->writers = 0;
    lock->readers_waiting = 0;
    lock->writers_waiting = 0;

    lock->kill = false;
}

void rwlock_uninit(struct RWLock* lock)
{
    lock->kill = true;

    do
    {
        if(lock->writers_waiting > 0)
        {
            cnd_broadcast(&lock->signal_w);
        }
        if(lock->readers_waiting > 0)
        {
            cnd_broadcast(&lock->signal_r);
        }
    } while(lock->writers > 0 || lock->readers > 0 || lock->readers_waiting > 0 || lock->writers_waiting > 0);

    cnd_destroy(&lock->signal_w);
    cnd_destroy(&lock->signal_r);
    mtx_destroy(&lock->lock);
}

bool rwlock_read_lock(struct RWLock* lock)
{
    if(lock->kill)
    {
        return false;
    }

    mtx_lock(&lock->lock);

    if(lock->writers == 1 || lock->writers_waiting > 0) // Prioritise writers
    {
        ++lock->readers_waiting;

        do
        {
            cnd_wait(&lock->signal_r, &lock->lock);
            if(lock->kill)
            {
                --lock->readers_waiting;
                mtx_unlock(&lock->lock);
                return false;
            }
        }
        while(lock->writers == 1 || lock->writers_waiting > 0);

        --lock->readers_waiting;
    }

    ++lock->readers;
    mtx_unlock(&lock->lock);
    return true;
}

void rwlock_read_unlock(struct RWLock* lock)
{
    mtx_lock(&lock->lock);

    --lock->readers;
    if(lock->writers_waiting > 0 && lock->readers == 0)
    {
        cnd_signal(&lock->signal_w);
    }

    mtx_unlock(&lock->lock);
}

bool rwlock_write_lock(struct RWLock* lock)
{
    if(lock->kill)
    {
        return false;
    }

    mtx_lock(&lock->lock);

    if(lock->writers == 1 || lock->readers > 0)
    {
        ++lock->writers_waiting;

        do
        {
            cnd_wait(&lock->signal_w, &lock->lock);
            if(lock->kill)
            {
                --lock->writers_waiting;
                mtx_unlock(&lock->lock);
                return false;
            }
        }
        while(lock->writers == 1 || lock->readers > 0);

        --lock->writers_waiting;
    }

    lock->writers = 1;
    mtx_unlock(&lock->lock);
    return true;
}

void rwlock_write_unlock(struct RWLock* lock)
{
    mtx_lock(&lock->lock);

    lock->writers = 0;

    if(lock->writers_waiting > 0)
    {
        cnd_signal(&lock->signal_w);
    }
    else if(lock->readers_waiting > 0)
    {
        cnd_broadcast(&lock->signal_r);
    }

    mtx_unlock(&lock->lock);
}

bool rwlock_lock(struct RWLock* lock, bool locktype)
{
    return locktype == WRITE ? rwlock_write_lock(lock) : rwlock_read_lock(lock);
}

void rwlock_unlock(struct RWLock* lock, bool locktype)
{
    if (locktype == WRITE)
    {
        rwlock_write_unlock(lock);
    }
    else
    {
        rwlock_read_unlock(lock);
    }
}

void rwlock_set_kill(struct RWLock* lock)
{
    lock->kill = true;
}

void rwlock_init_wrapper(void* lock, const void* args)
{
    struct RWLock* _lock = lock;
    rwlock_init(_lock);
}

void rwlock_uninit_wrapper(void* lock)
{
    struct RWLock* _lock = lock;
    rwlock_uninit(_lock);
}
