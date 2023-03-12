//#include "scieppend/core/hash_map.h"
//
//#include "scieppend/core/hash.h"
//
//static const C_HASH_MAP_DATA_CAPACITY_SCALAR  = 4;
//static const C_HASH_MAP_DEFAULT_BUCKETS_COUNT = 16;
//static const C_HASH_MAP_DEFAULT_DATA_CAPACITY = C_HASH_MAP_DEFAULT_BUCKETS_COUNT * C_HASH_MAP_DATA_CAPACITY_SCALAR;
//
//struct BucketData
//{
//    struct BucketData* next;
//    int data_idx;
//};
//
//struct Bucket
//{
//    int hashed_key;
//    int item_count;
//    struct BucketData* head;
//};
//
//static float _hash_map_load_factor(struct HashMap* map)
//{
//    return (float)map->size / (float)map->capacity;
//}
//
//static float _hash_map_rehash(struct HashMap* map)
//{
//    return 0.0f;
//}
//
//struct HashMap* hash_map_new(int item_size)
//{
//    struct HashMap* map = malloc(sizeof(struct HashMap));
//    map->item_size     = item_size;
//    map->item_count    = 0;
//    map->buckets_count = C_HASH_MAP_DEFAULT_BUCKET_COUNT;
//    map->buckets       = malloc(sizeof(struct Bucket) * C_HASH_MAP_DEFAULT_BUCKET_COUNT);
//    map->bucket_data   = malloc(sizeof(struct BucketData) * C_HASH_MAP_DEFAULT_DATA_CAPACITY);
//    map->data          = malloc(item_size * C_HASH_MAP_DEFAULT_DATA_CAPACITY);
//
//    for(int i = 0; i < C_HASH_MAP_DEFAULT_BUCKET_COUNT; ++i)
//    {
//        map->buckets[i].hashed_key = -1;
//        map->buckets[i].head       = NULL;
//        map->buckets[i].item_count = 0;
//    }
//
//    return map;
//}
//
//void hash_map_free(struct HashMap* map)
//{
//    free(map->data);
//    free(map->bucket_data);
//    free(map->buckets);
//    free(map);
//}
//
//void hash_map_add(struct HashMap* map, void* key, int key_bytes, void* item)
//{
//    int hashed_key = hash(key, key_bytes); 
//    int bucket_idx = hashed_key % map->buckets_count;
//
//    struct Bucket* bucket = &((struct Buckets*)map->buckets)[bucket_idx];
//
//    if(bucket->item_count == 0
//}
