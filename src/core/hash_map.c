#include "scieppend/core/hash_map.h"

#include "scieppend/core/hash.h"
#include "scieppend/core/link_array.h"

static const C_HASH_MAP_BUCKET_CAPACITY       = 4;
static const C_HASH_MAP_DEFAULT_BUCKETS_COUNT = 16;
static const C_HASH_MAP_DEFAULT_DATA_CAPACITY = C_HASH_MAP_DEFAULT_BUCKETS_COUNT * C_HASH_MAP_BUCKET_CAPACITY;
static float C_HASH_MAP_LOAD_FACTOR_THRESHOLD = 0.7f;

struct Bucket
{
    int              hashed_key;
    struct LinkArray data;
};

static float _hash_map_load_factor(struct HashMap* map)
{
    return (float)map->size / (float)map->buckets_count;
}

static float _hash_map_rehash(struct HashMap* map)
{
    return 0.0f;
}

struct HashMap* hash_map_new(int item_size)
{
    struct HashMap* map = malloc(sizeof(struct HashMap));
    hash_map_init(map, item_size);
    return map;
}

void hash_map_free(struct HashMap* map)
{
    free(map->data);
    free(map->bucket_data);
    free(map->buckets);
    free(map);
}

void hash_map_init(struct HashMap* map, int item_size)
{
    map->item_size     = item_size;
    map->item_count    = 0;
    map->buckets_count = C_HASH_MAP_DEFAULT_BUCKETS_COUNT;
    map->buckets       = malloc(sizeof(struct Bucket) * C_HASH_MAP_DEFAULT_BUCKETS_COUNT);

    for(int i = 0; i < C_HASH_MAP_DEFAULT_BUCKETS_COUNT; ++i)
    {
        struct Bucket* bucket = &((struct Bucket*)map->buckets)[i];
        bucket->hashed_key = -1;
        link_array_init(&bucket->data, item_size, C_HASH_MAP_BUCKET_CAPACITY);
    }
}

void hash_map_uninit(struct HashMap* map)
{
    for(int i = 0; i < map->bucket_count; ++i)
    {
        struct Bucket* bucket = &((struct Bucket*)map->buckets)[i];
        link_array_uninit(&bucket->data);
    }

    free(map->buckets);
}

int hash_map_count(struct HashMap* map)
{
    return map->item_count;
}

int hash_map_capacity(struct HashMap* map)
{
    int capacity = 0;

    for(int i = 0; i < map->buckets_count; ++i)
    {
        struct Bucket* bucket = &((struct Bucket*)map->buckets)[i];
        capacity += bucket->data.capacity;
    }

    return capacity;
}

void hash_map_add(struct HashMap* map, void* key, int key_bytes, void* item)
{
    float load_factor = _hash_map_load_factor(map);    
    if(load_factor > C_HASH_MAP_LOAD_FACTOR_THRESHOLD)
    {
        // Make bigger
    }

    int hashed_key = hash(key, key_bytes); 
    int bucket_idx = hashed_key & (map->buckets_count - 1);

    struct Bucket* bucket = &((struct Bucket*)map->buckets)[bucket_idx];
    if(bucket->data.count == bucket->data.capacity)
    {
        // Need to resize hash map?
    }

    link_array_add(&bucket->data, item);
}

void* hash_map_get(struct HashMap* map, void* key, int key_bytes)
{
    int hashed_key = hash(key, key_bytes);
    int bucket_idx = hashed_key & (map->buckets_count - 1);

    struct Bucket* bucket = &((struct Bucket*)map->buckets)[bucket_idx];
}
