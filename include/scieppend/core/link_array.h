#ifndef SCIEPPEND_CORE_LINK_ARRAY_H
#define SCIEPPEND_CORE_LINK_ARRAY_H

#include "scieppend/core/container_common.h"

#include <stdbool.h>

struct LinkArrayNode
{
    int   next;
    int   prev;
    void* data;
};

struct LinkArray
{
    int item_size;
    int count;
    int capacity;
    int usedhead;
    int usedtail;
    int freehead;
    alloc_fn alloc;
    free_fn  free;
    void* nodes;
};

struct LinkArrayIt
{
    struct LinkArray* array;
    int               index;
};

struct LinkArray* linkarray_new(int item_size, int capacity, alloc_fn alloc, free_fn free);
void linkarray_free(struct LinkArray* array);
void linkarray_init(struct LinkArray* array, int item_size, int capacity, alloc_fn alloc, free_fn free);
void linkarray_uninit(struct LinkArray* array);

// Accessors
int    linkarray_count(struct LinkArray* array);
void*  linkarray_front_voidp(struct LinkArray* array);
void*  linkarray_back_voidp(struct LinkArray* array);
void*  linkarray_at_voidp(struct LinkArray* array, int index);

#define linkarray_front(array, type) ((type) *((type*)linkarray_front_voidp((array))))
#define linkarray_back(array, type) ((type) *((type*)linkarray_back_voidp((array))))
#define linkarray_at(array, index, type) ((type) *((type*)linkarray_at_voidp((array), (index))))

// Modifiers
void linkarray_push_front(struct LinkArray* array, void* item);
void linkarray_push_back(struct LinkArray* array, void* item);
void linkarray_clear(struct LinkArray* array);
void linkarray_pop_front(struct LinkArray* array);
void linkarray_pop_back(struct LinkArray* array);
void linkarray_pop_at(struct LinkArray* array, int index);
void linkarray_emplace_back(struct LinkArray* array, void* args);

// Iterator functions

struct LinkArrayIt linkarray_begin(struct LinkArray* array);
struct LinkArrayIt linkarray_end(struct LinkArray* array);
bool               linkarray_it_eq(struct LinkArrayIt lhs, struct LinkArrayIt rhs);
struct LinkArrayIt linkarray_it_next(struct LinkArrayIt it);
void*              linkarray_it_get_voidp(struct LinkArrayIt it);

#define linkarray_it_get(it, type) ((type) *((type*)linkarray_it_get_voidp((it))))

#endif
