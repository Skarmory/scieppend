#ifndef SCIEPPEND_CORE_DYNAMIC_ARRAY_H
#define SCIEPPEND_CORE_DYNAMIC_ARRAY_H

#include "scieppend/core/container_common.h"

#include <stdbool.h>

/* Dynamically sized array.
 */

struct Array
{
    char*    data;
    int      count;
    int      capacity;
    int      item_size;
    alloc_fn alloc_func;
    free_fn  free_func;
};

struct Array* array_new(int item_size, int capacity, alloc_fn alloc_func, free_fn free_func);
void          array_init(struct Array* array, int item_size, int capacity, alloc_fn alloc_func, free_fn free_func);
void          array_free(struct Array* array);
void          array_uninit(struct Array* array);
void          array_copy(struct Array* dst, struct Array* src);
void          array_move(struct Array* dst, struct Array* src);
int           array_count(const struct Array* array);
int           array_capacity(struct Array* array);
void          array_add(struct Array* array, void* item);
void*         array_emplace(struct Array* array, void* args);
void          array_remove_at(struct Array* array, int index);
void*         array_get(const struct Array* array, int index);
void          array_shrink(struct Array* array);
int           array_find(const struct Array*, const void* item, compare_fn comp_func);
bool          array_find_and_remove(struct Array*, const void* item, compare_fn comp_func);
int           array_find_sorted(const struct Array*, const void* item, compare_fn comp_func);
void          array_sort(struct Array* array, compare_fn comp_func);
void          array_clear(struct Array* array);

#endif
