#include "scieppend/core/hash_map.h"

#include "scieppend/core/hash.h"
#include "scieppend/core/link_array.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const int   C_HASH_MAP_BUCKET_CAPACITY      = 16;
static const int   C_HASH_MAP_DEFAULT_BUCKET_COUNT = 16;
static const int   C_HASH_MAP_DEFAULT_BUCKET_ITEMS_CAPACITY = C_HASH_MAP_DEFAULT_BUCKET_COUNT * C_HASH_MAP_BUCKET_CAPACITY;
static const float C_HASH_MAP_LOAD_FACTOR_THRESHOLD = 0.7f;

struct HashMapBucketItemArgs
{
    int   key;
    void* next;
    void* data;
    int   data_size;
};

// Calculate the aligned data size of a HashMapBucketItem
// Note: this assumes that data_size has a natural alignment of <= 8 bytes
static inline int _bucket_item_size_aligned(int data_size)
{
    return offsetof(struct HashMapBucketItem, data) + data_size;
}

static inline void* _get_bucket_item_data(struct HashMapBucketItem* item)
{
    return (char*)item + offsetof(struct HashMapBucketItem, data);
}

static inline void _init_hash_map_buckets(struct HashMap* map)
{
    for(int i = 0; i < map->bucket_count; ++i)
    {
        struct HashMapBucket* bucket = &map->buckets[i];
        bucket->count = 0;
        bucket->items = NULL;
    }
}

static inline float _hash_map_load_factor(struct HashMap* map)
{
    return (float)hash_map_count(map) / (float)map->bucket_count;
}

static inline bool _hash_map_need_resize(struct HashMap* map)
{
    return _hash_map_load_factor(map) > C_HASH_MAP_LOAD_FACTOR_THRESHOLD;
}

static void _hash_map_resize(struct HashMap* map)
{
    int new_bucket_count = map->bucket_count << 1;

    map->buckets = realloc(map->buckets, sizeof(struct HashMapBucket) * new_bucket_count);
    if(!map->buckets)
    {
        abort();
    }

    map->bucket_count = new_bucket_count;

    _init_hash_map_buckets(map);

    for(struct LinkArrayIt it = linkarray_begin(&map->bucket_items); !linkarray_it_eq(it, linkarray_end(&map->bucket_items)); it = linkarray_it_next(it))
    {
        struct HashMapBucketItem* bucket_item = linkarray_it_get_voidp(it);

        int bucket_idx = bucket_item->key & (map->bucket_count - 1);

        struct HashMapBucket* bucket = &map->buckets[bucket_idx];
        bucket_item->next = bucket->items;
        bucket->items = bucket_item;
        ++bucket->count;
    }
}

static void _hash_map_bucket_item_alloc(void* item, void* args)
{
    struct HashMapBucketItem* bucket_item = item;
    struct HashMapBucketItemArgs* bucket_args = args;

    bucket_item->key = bucket_args->key;
    bucket_item->next = bucket_args->next;
    memcpy(_get_bucket_item_data(bucket_item), bucket_args->data, bucket_args->data_size);
}

struct HashMap* hash_map_new(int item_size)
{
    struct HashMap* map = malloc(sizeof(struct HashMap));
    hash_map_init(map, item_size);
    return map;
}

void hash_map_free(struct HashMap* map)
{
    hash_map_uninit(map);
    free(map);
}

void hash_map_init(struct HashMap* map, int item_size)
{
    map->item_size    = item_size;
    map->bucket_count = C_HASH_MAP_DEFAULT_BUCKET_COUNT;
    map->buckets      = malloc(sizeof(struct HashMapBucket) * C_HASH_MAP_DEFAULT_BUCKET_COUNT);

    linkarray_init(&map->bucket_items, _bucket_item_size_aligned(item_size), C_HASH_MAP_DEFAULT_BUCKET_ITEMS_CAPACITY, &_hash_map_bucket_item_alloc, NULL);

    _init_hash_map_buckets(map);
}

void hash_map_uninit(struct HashMap* map)
{
    linkarray_uninit(&map->bucket_items);
    free(map->buckets);

    map->buckets      = NULL;
    map->item_size    = 0;
    map->bucket_count = 0;
}

int hash_map_count(struct HashMap* map)
{
    return linkarray_count(&map->bucket_items);
}

void hash_map_add(struct HashMap* map, void* key, int key_bytes, void* item)
{
    if(_hash_map_need_resize(map))
    {
        _hash_map_resize(map);
    }

    int hashed_key = hash(key, key_bytes);
    int bucket_idx = hashed_key & (map->bucket_count - 1);

    struct HashMapBucket* bucket = &map->buckets[bucket_idx];
    if(bucket->count == C_HASH_MAP_BUCKET_CAPACITY)
    {
        // Need to resize hash map?
    }

    struct HashMapBucketItemArgs bucket_item;
    bucket_item.key = hashed_key;
    bucket_item.next = bucket->items;
    bucket_item.data = item;
    bucket_item.data_size = map->item_size;

    linkarray_emplace_back(&map->bucket_items, &bucket_item);

    struct HashMapBucketItem* actual_bucket_item = (struct HashMapBucketItem*)linkarray_back_voidp(&map->bucket_items);
    bucket->items = actual_bucket_item;

    ++bucket->count;
}

void* hash_map_get_voidp(struct HashMap* map, void* key, int key_bytes)
{
    int hashed_key = hash(key, key_bytes);
    int bucket_idx = hashed_key & (map->bucket_count - 1);

    struct HashMapBucket* bucket = &map->buckets[bucket_idx];
    struct HashMapBucketItem* bucket_item = bucket->items;
    while(bucket_item != NULL && bucket_item->key != hashed_key)
    {
        bucket_item = bucket_item->next;
    }

    return _get_bucket_item_data(bucket_item);
}
