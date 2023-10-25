#include "scieppend/core/array_threadsafe.h"

#include <stdlib.h>
#include <string.h>

struct Array_ThreadSafe* array_ts_new(int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    struct Array_ThreadSafe* array = malloc(sizeof(struct Array_ThreadSafe));
    array_ts_init(array, item_size, capacity, alloc_func, free_func);
    return array;
}

void array_ts_init(struct Array_ThreadSafe* array, int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    rwlock_init(&array->lock);
    rwlock_write_lock(&array->lock);
    array_init(&array->array, item_size, capacity, alloc_func, free_func);
    rwlock_write_unlock(&array->lock);
}

void array_ts_free(struct Array_ThreadSafe* array)
{
    array_ts_uninit(array);
    free(array);
}

void array_ts_uninit(struct Array_ThreadSafe* array)
{
    rwlock_write_lock(&array->lock);
    array_uninit(&array->array);
    rwlock_write_unlock(&array->lock);
    rwlock_uninit(&array->lock);
}

void array_ts_copy(struct Array_ThreadSafe* restrict dst, struct Array_ThreadSafe* restrict src)
{
    rwlock_write_lock(&dst->lock);
    rwlock_write_lock(&src->lock);

    array_copy(&dst->array, &src->array);

    rwlock_write_unlock(&src->lock);
    rwlock_write_unlock(&dst->lock);
}

void array_ts_move(struct Array_ThreadSafe* restrict dst, struct Array_ThreadSafe* restrict src)
{
    rwlock_write_lock(&dst->lock);
    rwlock_write_lock(&src->lock);

    array_move(&dst->array, &src->array);

    rwlock_write_unlock(&src->lock);
    rwlock_write_unlock(&dst->lock);
}

int array_ts_count(const struct Array_ThreadSafe* array)
{
    rwlock_read_lock(&array->lock);
    int ret = array_count(&array->array);
    rwlock_read_unlock(&array->lock);
    return ret;
}

int array_ts_capacity(struct Array_ThreadSafe* array)
{
    rwlock_read_lock(&array->lock);
    int ret = array_capacity(&array->array);
    rwlock_read_unlock(&array->lock);
    return ret;
}

void array_ts_add(struct Array_ThreadSafe* array, void* item)
{
    rwlock_write_lock(&array->lock);
    array_add(&array->array, item);
    rwlock_write_unlock(&array->lock);
}

void* array_ts_emplace(struct Array_ThreadSafe* array, void* args)
{
    rwlock_write_lock(&array->lock);
    void* ret = array_emplace(&array->array, args);
    rwlock_write_unlock(&array->lock);
    return ret;
}

void array_ts_remove_at(struct Array_ThreadSafe* array, int index)
{
    rwlock_write_lock(&array->lock);
    array_remove_at(&array->array, index);
    rwlock_write_unlock(&array->lock);
}

void* array_ts_get(const struct Array_ThreadSafe* array, int index)
{
    rwlock_read_lock(&array->lock);
    void* ret = array_get(&array->array, index);
    rwlock_read_unlock(&array->lock);
    return ret;
}

void array_ts_shrink(struct Array_ThreadSafe* array)
{
    rwlock_write_lock(&array->lock);
    array_shrink(&array->array);
    rwlock_write_unlock(&array->lock);
}

int array_ts_find(const struct Array_ThreadSafe* array, const void* item, compare_fn comp_func)
{
    rwlock_read_lock(&array->lock);
    int ret = array_find(&array->array, item, comp_func);
    rwlock_read_unlock(&array->lock);
    return ret;
}

int array_ts_find_sorted(const struct Array_ThreadSafe* array, const void* item, compare_fn comp_func)
{
    rwlock_read_lock(&array->lock);
    int ret = array_find_sorted(&array->array, item, comp_func);
    rwlock_read_unlock(&array->lock);
    return ret;
}

void array_ts_sort(struct Array_ThreadSafe* array, compare_fn comp_func)
{
    rwlock_write_lock(&array->lock);
    array_sort(&array->array, comp_func);
    rwlock_write_unlock(&array->lock);
}

void* array_ts_find_and_get(struct Array_ThreadSafe* array, const void* item, compare_fn comp_func)
{
    void* ret = NULL;

    rwlock_read_lock(&array->lock);
    {
        int idx = array_find(&array->array, item, comp_func);
        if(idx != -1)
        {
            ret = array_get(&array->array, idx);
        }
    }
    rwlock_read_unlock(&array->lock);

    return ret;
}

void array_ts_find_and_remove(struct Array_ThreadSafe* array, const void* item, compare_fn compare_func)
{
    rwlock_write_lock(&array->lock);
    {
        int idx = array_find(&array->array, item, compare_func);
        if(idx != -1)
        {
            array_remove_at(&array->array, idx);
        }
    }
    rwlock_write_unlock(&array->lock);
}
