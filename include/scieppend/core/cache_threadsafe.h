#ifndef SCIEPPEND_CORE_CACHE_THREADSAFE_H
#define SCIEPPEND_CORE_CACHE_THREADSAFE_H

#include "scieppend/core/cache.h"
#include "scieppend/core/iterator.h"
#include "scieppend/core/rw_lock.h"

struct Cache_ThreadSafe
{
    struct Cache cache;
    struct RWLock lock;
};

struct Cache_ThreadSafe* cache_ts_new(int item_size, int capacity, alloc_fn alloc_func, free_fn free_func);
void cache_ts_init(struct Cache_ThreadSafe* cache, int item_size, int capacity, alloc_fn alloc_func, free_fn free_func);
void cache_ts_free(struct Cache_ThreadSafe* cache);
void cache_ts_uninit(struct Cache_ThreadSafe* cache);
int cache_ts_count(const struct Cache_ThreadSafe* cache);
int cache_ts_capacity(const struct Cache_ThreadSafe* cache);
int cache_ts_item_size(const struct Cache_ThreadSafe* cache);
int cache_ts_used(const struct Cache_ThreadSafe* cache);
bool cache_ts_stale_handle(const struct Cache_ThreadSafe* cache, int handle);
int cache_ts_add(struct Cache_ThreadSafe* cache, const void* item);
int cache_ts_emplace(struct Cache_ThreadSafe* cache, void* args);
void cache_ts_remove(struct Cache_ThreadSafe* cache, int handle);
void* cache_ts_get(const struct Cache_ThreadSafe* cache, int handle);

bool cache_ts_lock(const struct Cache_ThreadSafe* cache, bool write);
void cache_ts_unlock(const struct Cache_ThreadSafe* cache, bool write);

#endif
