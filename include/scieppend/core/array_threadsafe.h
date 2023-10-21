#ifndef SCIEPPEND_CORE_ARRAY_THREADSAFE_H
#define SCIEPPEND_CORE_ARRAY_THREADSAFE_H

#include <stdatomic.h>

#include "scieppend/core/array.h"
#include "scieppend/core/rw_lock.h"

struct Array_ThreadSafe
{
    struct Array  array;
    struct RWLock lock;
};

struct Array_ThreadSafe* array_ts_new(int item_size, int capacity, alloc_fn alloc_func, free_fn free_func);
void  array_ts_init(struct Array_ThreadSafe* array, int item_size, int capacity, alloc_fn alloc_func, free_fn free_func);
void  array_ts_free(struct Array_ThreadSafe* array);
void  array_ts_uninit(struct Array_ThreadSafe* array);
void  array_ts_copy(struct Array_ThreadSafe* dst, struct Array_ThreadSafe* src);
void  array_ts_move(struct Array_ThreadSafe* dst, struct Array_ThreadSafe* src);
int   array_ts_count(const struct Array_ThreadSafe* array);
int   array_ts_capacity(struct Array_ThreadSafe* array);
void  array_ts_add(struct Array_ThreadSafe* array, void* item);
void* array_ts_emplace(struct Array_ThreadSafe* array, void* args);
void  array_ts_remove_at(struct Array_ThreadSafe* array, int index);
void* array_ts_get(const struct Array_ThreadSafe* array, int index);
void  array_ts_shrink(struct Array_ThreadSafe* array);
int   array_ts_find(const struct Array_ThreadSafe*, const void* item, compare_fn comp_func);
int   array_ts_find_sorted(const struct Array_ThreadSafe*, const void* item, compare_fn comp_func);
void  array_ts_sort(struct Array_ThreadSafe* array, compare_fn comp_func);

#endif

