#include "scieppend/core/rw_lock.h"

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
