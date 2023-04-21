#ifndef SCIEPPEND_CORE_HASH_MAP_H
#define SCIEPPEND_CORE_HASH_MAP_H

#include "scieppend/core/array.h"
#include "scieppend/core/link_array.h"

struct HashMapBucketItem
{
    int                       key;
    struct HashMapBucketItem* next;
    void*                     data;
};

struct HashMapBucket
{
    int count;
    struct HashMapBucketItem* items;
};

struct HashMap
{
    int item_size;
    int bucket_count;

    struct HashMapBucket* buckets;
    struct LinkArray      bucket_items;
};

struct HashMap* hash_map_new(int item_size);
void            hash_map_free(struct HashMap* map);
void            hash_map_init(struct HashMap* map, int item_size);
void            hash_map_uninit(struct HashMap* map);

int hash_map_count(struct HashMap* map);

void  hash_map_add(struct HashMap* map, void* key, int key_bytes, void* item);
void* hash_map_get_voidp(struct HashMap* map, void* key, int key_bytes);
#define hash_map_get(map, key, key_bytes, type) *(type*)hash_map_get_voidp((map), (key), (key_bytes))

#endif
