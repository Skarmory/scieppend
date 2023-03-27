#ifndef SCIEPPEND_CORE_HASH_MAP_H
#define SCIEPPEND_CORE_HASH_MAP_H

struct HashMap
{
    int   item_size;
    int   item_count;
    int   buckets_count;
    void* buckets;
};

struct HashMap* hash_map_new(int item_size);
void            hash_map_free(struct HashMap* map);
void            hash_map_init(struct HashMap* map, int item_size);
void            hash_map_uninit(struct HashMap* map);

int hash_map_count(struct HashMap* map);
int hash_map_capacity(struct HashMap* map);

void  hash_map_add(struct HashMap* map, void* key, int key_bytes, void* item);
void* hash_map_get(struct HashMap* map, void* key, int key_bytes);

#endif
