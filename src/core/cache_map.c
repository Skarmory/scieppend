#include "scieppend/core/cache_map.h"

#include "scieppend/core/hash.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const int C_DEFAULT_BUCKET_COUNT = 16;
static const int C_DEFAULT_BUCKET_CAPACITY = 16;
static const float C_CACHE_MAP_LOAD_FACTOR_THRESHOLD = 0.75f;

// INTERNAL FUNCS

static inline int _get_bucket_index(int hashed_key, int bucket_count)
{
    return hashed_key & (bucket_count - 1);
}

static inline int _get_bucket_item_index(int bucket_index, int item_offset, int bucket_capacity)
{
    return (bucket_index * bucket_capacity) + item_offset;
}

// Search a bucket for the item
static inline struct CacheMapBucketItem* _get_bucket_item(struct CacheMapBucketItem* buckets, int hashed_key, int bucket_count, int bucket_capacity)
{
    int bucket_idx = _get_bucket_index(hashed_key, bucket_count);
    for(int item_idx = 0; item_idx < bucket_capacity; ++item_idx)
    {
        struct CacheMapBucketItem* item = &buckets[_get_bucket_item_index(bucket_idx, item_idx, bucket_capacity)];
        if(item->key == hashed_key)
        {
            return item;
        }
    }

    return NULL;
}

static inline struct CacheMapBucketItem* _get_next_bucket_item(struct CacheMapBucketItem* buckets, int hashed_key, int bucket_count, int bucket_capacity)
{
    // First check if this key already exists, if it does then we must reuse it.
    struct CacheMapBucketItem* item = _get_bucket_item(buckets, hashed_key, bucket_count, bucket_capacity);
    if (item != NULL)
    {
        return item;
    }

    // Key doesn't exist, so find a free spot
    int bucket_idx = _get_bucket_index(hashed_key, bucket_count);
    for(int item_idx = 0; item_idx < bucket_capacity; ++item_idx)
    {
        struct CacheMapBucketItem* item = &buckets[_get_bucket_item_index(bucket_idx, item_idx, bucket_capacity)];
        if(item->handle == C_NULL_CACHE_HANDLE)
        {
            return item;
        }
    }

    return NULL;
}

static inline float _load_factor(int item_count, int bucket_count)
{
    return (float)item_count / (float)bucket_count;
}

static inline void _init_bucket_items(struct CacheMapBucketItem* buckets, int bucket_count, int bucket_capacity)
{
    for(int i = 0; i < bucket_count * bucket_capacity; ++i)
    {
        struct CacheMapBucketItem* item = &buckets[i];
        item->key = -1;
        item->handle = C_NULL_CACHE_HANDLE;
    }
}

static inline bool _need_resize(struct CacheMap* map)
{
    return _load_factor(cache_size(&map->bucket_items), map->bucket_count) > C_CACHE_MAP_LOAD_FACTOR_THRESHOLD;
}

static void _resize(struct CacheMap* map)
{
    int new_bucket_count = map->bucket_count << 1;

    struct CacheMapBucketItem* new_buckets = malloc(new_bucket_count * sizeof(struct CacheMapBucketItem));

    _init_bucket_items(new_buckets, new_bucket_count, map->bucket_capacity);

    // Rehash
    for(int i = 0; i < map->bucket_count * map->bucket_capacity; ++i)
    {
        struct CacheMapBucketItem* old_item = &map->buckets[i];
        struct CacheMapBucketItem* new_item = _get_next_bucket_item(new_buckets, old_item->key, new_bucket_count, map->bucket_capacity);
        *new_item = *old_item;
    }

    free(map->buckets);
    map->buckets = new_buckets;
    map->bucket_count = new_bucket_count;
}

// EXTERNAL FUNCS

// CREATIONAL

struct CacheMap* cache_map_new(int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    struct CacheMap* map = malloc(sizeof(struct CacheMap));
    cache_map_init(map, item_size, capacity, alloc_func, free_func);
    return map;
}

void cache_map_free(struct CacheMap* map)
{
    cache_map_uninit(map);
    free(map);
}

void cache_map_init(struct CacheMap* map, int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    map->item_size = item_size;
    map->bucket_count = C_DEFAULT_BUCKET_COUNT;
    map->bucket_capacity = C_DEFAULT_BUCKET_CAPACITY;
    map->buckets = malloc(map->bucket_capacity * map->bucket_count * sizeof(struct CacheMapBucketItem));
    _init_bucket_items(map->buckets, map->bucket_count, map->bucket_capacity);
    cache_init(&map->bucket_items, map->item_size, capacity, alloc_func, free_func);
}

void cache_map_uninit(struct CacheMap* map)
{
    cache_uninit(&map->bucket_items);
    free(map->buckets);
    map->bucket_count = 0;
    map->item_size = 0;
}

// ACCESSORS

int cache_map_count(const struct CacheMap* map)
{
    return cache_size(&map->bucket_items);
}

void* cache_map_get(const struct CacheMap* map, const void* key, int key_bytes)
{
    int hashed_key = hash(key, key_bytes);
    return cache_map_get_hashed(map, hashed_key);
}

void* cache_map_get_hashed(const struct CacheMap* map, int hashed_key)
{
    struct CacheMapBucketItem* bucket_item = _get_bucket_item(map->buckets, hashed_key, map->bucket_count, map->bucket_capacity);

    if(bucket_item == NULL)
    {
        return NULL;
    }

    void* data = cache_get(&map->bucket_items, bucket_item->handle);

    return data;
}

float cache_map_load_factor(const struct CacheMap* map)
{
    return _load_factor(cache_size(&map->bucket_items), map->bucket_count) > C_CACHE_MAP_LOAD_FACTOR_THRESHOLD;
}

struct It cache_map_begin(struct CacheMap* map)
{
    struct It it;
    it.container = map;

    for(it.index = 0; it.index < (map->bucket_count * map->bucket_capacity); ++it.index)
    {
        struct CacheMapBucketItem* item = &map->buckets[it.index];
        if(item->handle != C_NULL_CACHE_HANDLE)
        {
            break;
        }
    }

    return it;
}

struct It cache_map_end(struct CacheMap* map)
{
    struct It it;
    it.container = map;
    it.index = map->bucket_count * map->bucket_capacity;
    return it;
}

void cache_map_it_next(struct It* it)
{
    struct CacheMap* map = it->container;

    for(++it->index; it->index < (map->bucket_count * map->bucket_capacity); ++it->index)
    {
        struct CacheMapBucketItem* item = &map->buckets[it->index];
        if(item->handle != C_NULL_CACHE_HANDLE)
        {
            return;
        }
    }
}

void* cache_map_it_get(const struct It* it)
{
    struct CacheMap* map = it->container;
    struct CacheMapBucketItem* bucket_item = &map->buckets[it->index];
    void* data = cache_get(&map->bucket_items, bucket_item->handle);
    return data;
}

// MUTATORS

void cache_map_add(struct CacheMap* map, const void* key, int key_bytes, const void* item)
{
    if(_need_resize(map))
    {
        _resize(map);
    }

    int item_h = cache_add(&map->bucket_items, item);
    int hashed_key = hash(key, key_bytes);

    struct CacheMapBucketItem* new_bucket_item = _get_next_bucket_item(map->buckets, hashed_key, map->bucket_count, map->bucket_capacity);
    if(new_bucket_item == NULL)
    {
        _resize(map);
        new_bucket_item = _get_next_bucket_item(map->buckets, hashed_key, map->bucket_count, map->bucket_capacity);
    }

    new_bucket_item->key = hashed_key;
    new_bucket_item->handle = item_h;
}

void* cache_map_emplace(struct CacheMap* map, const void* key, int key_bytes, const void* args)
{
    if(_need_resize(map))
    {
        _resize(map);
    }

    int hashed_key = hash(key, key_bytes);
    int item_h = cache_emplace(&map->bucket_items, args);

    struct CacheMapBucketItem* new_bucket_item = _get_next_bucket_item(map->buckets, hashed_key, map->bucket_count, map->bucket_capacity);
    if(new_bucket_item == NULL)
    {
        _resize(map);
        new_bucket_item = _get_next_bucket_item(map->buckets, hashed_key, map->bucket_count, map->bucket_capacity);
    }

    new_bucket_item->key = hashed_key;
    new_bucket_item->handle = item_h;

    return cache_map_get_hashed(map, hashed_key);
}

void* cache_map_emplace_hashed(struct CacheMap* map, const int hashed_key, const void* args)
{
    if(_need_resize(map))
    {
        _resize(map);
    }

    int item_h = cache_emplace(&map->bucket_items, args);

    struct CacheMapBucketItem* new_bucket_item = _get_next_bucket_item(map->buckets, hashed_key, map->bucket_count, map->bucket_capacity);
    if(new_bucket_item == NULL)
    {
        _resize(map);
        new_bucket_item = _get_next_bucket_item(map->buckets, hashed_key, map->bucket_count, map->bucket_capacity);
    }

    new_bucket_item->key = hashed_key;
    new_bucket_item->handle = item_h;

    return cache_map_get_hashed(map, hashed_key);
}

void cache_map_remove(struct CacheMap* map, const void* key, int key_bytes)
{
    int hashed_key = hash(key, key_bytes);
    struct CacheMapBucketItem* bucket_item = _get_bucket_item(map->buckets, hashed_key, map->bucket_count, map->bucket_capacity);
    if(!bucket_item)
    {
        return;
    }

    cache_remove(&map->bucket_items, bucket_item->handle);

    bucket_item->handle = C_NULL_CACHE_HANDLE;
    bucket_item->key = -1;
}
