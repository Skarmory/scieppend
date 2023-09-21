#ifndef SCIEPPEND_CORE_CACHE_MAP_H
#define SCIEPPEND_CORE_CACHE_MAP_H

#include "scieppend/core/cache.h"
#include "scieppend/core/iterator.h"

struct CacheMapBucketItem
{
    int key;
    int handle;
};

struct CacheMap
{
    int item_size;
    int bucket_count;
    int bucket_capacity;

    struct CacheMapBucketItem* buckets;
    struct Cache bucket_items;
};

// CREATIONAL

struct CacheMap* cache_map_new(int item_size);
void             cache_map_free(struct CacheMap* map);
void             cache_map_init(struct CacheMap* map, int item_size);
void             cache_map_uninit(struct CacheMap* map);

// ACCESSORS

int     cache_map_count(struct CacheMap* map);
void*   cache_map_get(struct CacheMap* map, const void* key, int key_bytes);
float   cache_map_load_factor(struct CacheMap* map);

struct It cache_map_begin(struct CacheMap* map);
struct It cache_map_end(struct CacheMap* map);
void      cache_map_it_next(struct It* it);
void*     cache_map_it_get(const struct It* it);

// MUTATORS

void  cache_map_add(struct CacheMap* map, const void* key, int key_bytes, const void* item);
void  cache_map_remove(struct CacheMap* map, const void* key, int key_bytes);

#endif
