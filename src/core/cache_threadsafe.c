#include "scieppend/core/cache_threadsafe.h"

#include <stdlib.h>

struct Cache_ThreadSafe* cache_ts_new(int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    struct Cache_ThreadSafe* cache = malloc(sizeof(struct Cache_ThreadSafe));
    cache_ts_init(cache, item_size, capacity, alloc_func, free_func);
    return cache;
}

void cache_ts_init(struct Cache_ThreadSafe* cache, int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    rwlock_init(&cache->lock);
    rwlock_write_lock(&cache->lock);
    cache_init(&cache->cache, item_size, capacity, alloc_func, free_func);
    rwlock_write_unlock(&cache->lock);
}

void cache_ts_free(struct Cache_ThreadSafe* cache)
{
    cache_ts_uninit(cache);
    free(cache);
}

void cache_ts_uninit(struct Cache_ThreadSafe* cache)
{
    rwlock_write_lock(&cache->lock);
    cache_uninit(&cache->cache);
    rwlock_write_unlock(&cache->lock);
    rwlock_uninit(&cache->lock);
}

int cache_ts_count(const struct Cache_ThreadSafe* cache)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    rwlock_read_lock(&cache->lock);
    int ret = cache_size(&cache->cache);
    rwlock_read_unlock(&cache->lock);
    return ret;
#pragma GCC diagnostic pop
}

int cache_ts_capacity(const struct Cache_ThreadSafe* cache)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    rwlock_read_lock(&cache->lock);
    int ret = cache_capacity(&cache->cache);
    rwlock_read_unlock(&cache->lock);
    return ret;
#pragma GCC diagnostic pop
}

int cache_ts_item_size(const struct Cache_ThreadSafe* cache)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    rwlock_read_lock(&cache->lock);
    int ret = cache_item_size(&cache->cache);
    rwlock_read_unlock(&cache->lock);
    return ret;
#pragma GCC diagnostic pop
}

int cache_ts_used(const struct Cache_ThreadSafe* cache)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    rwlock_read_lock(&cache->lock);
    int ret = cache_used(&cache->cache);
    rwlock_read_unlock(&cache->lock);
    return ret;
#pragma GCC diagnostic pop
}

bool cache_ts_stale_handle(const struct Cache_ThreadSafe* cache, int handle)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    rwlock_read_lock(&cache->lock);
    bool ret = cache_stale_handle(&cache->cache, handle);
    rwlock_read_unlock(&cache->lock);
    return ret;
#pragma GCC diagnostic pop
}

int cache_ts_add(struct Cache_ThreadSafe* cache, const void* item)
{
    rwlock_write_lock(&cache->lock);
    int ret = cache_add(&cache->cache, item);
    rwlock_write_unlock(&cache->lock);
    return ret;
}

int cache_ts_emplace(struct Cache_ThreadSafe* cache, void* args)
{
    rwlock_write_lock(&cache->lock);
    int ret = cache_emplace(&cache->cache, args);
    rwlock_write_unlock(&cache->lock);
    return ret;
}

void cache_ts_remove(struct Cache_ThreadSafe* cache, int handle)
{
    rwlock_write_lock(&cache->lock);
    cache_remove(&cache->cache, handle);
    rwlock_write_unlock(&cache->lock);
}

void* cache_ts_get(const struct Cache_ThreadSafe* cache, int handle)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    rwlock_read_lock(&cache->lock);
    void* ret = cache_get(&cache->cache, handle);
    rwlock_read_unlock(&cache->lock);
    return ret;
#pragma GCC diagnostic pop
}

bool cache_ts_lock(const struct Cache_ThreadSafe* cache, bool write)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    return write ? rwlock_write_lock(&cache->lock) : rwlock_read_lock(&cache->lock);
#pragma GCC diagnostic pop
}

void cache_ts_unlock(const struct Cache_ThreadSafe* cache, bool write)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    if(write)
    {
        rwlock_write_unlock(&cache->lock);
    }
    else
    {
        rwlock_read_unlock(&cache->lock);
    }
#pragma GCC diagnostic pop
}
