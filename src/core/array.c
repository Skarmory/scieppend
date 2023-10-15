#include "scieppend/core/array.h"

#include <stdlib.h>
#include <string.h>

static const int C_DEFAULT_CAPACITY = 8;

static void _array_realloc(struct Array* array, int new_capacity)
{
    array->data = realloc(array->data, array->item_size * new_capacity);

    if(!array->data)
    {
        abort();
    }

    array->capacity = new_capacity;
}

static char* _array_next_element(struct Array* array)
{
    if(array->count == array->capacity)
    {
        _array_realloc(array, array->capacity << 1);
    }

    return array->data + (array->item_size * array->count);
}

struct Array* array_new(int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    struct Array* array = malloc(sizeof(struct Array));
    array_init(array, item_size, capacity, alloc_func, free_func);
    return array;
}

void array_init(struct Array* array, int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    array->capacity   = capacity > 0 ? capacity : C_DEFAULT_CAPACITY;
    array->count      = 0;
    array->item_size  = item_size;
    array->data       = malloc(item_size * array->capacity);
    array->alloc_func = alloc_func;
    array->free_func  = free_func;
}

void array_free(struct Array* array)
{
    array_uninit(array);
    free(array);
}

void array_uninit(struct Array* array)
{
    if(array->free_func)
    {
        for(int i = 0; i < array->count; ++i)
        {
            array->free_func(array_get(array, i));
        }
    }

    free(array->data);
}

void array_copy(struct Array* restrict dst, struct Array* restrict src)
{
    array_init(dst, src->item_size, src->count, src->alloc_func, src->free_func);
    memcpy(dst->data, src->data, src->item_size * src->count);
    dst->count = src->count;
}

void array_move(struct Array* restrict dst, struct Array* restrict src)
{
    dst->data = src->data;
    dst->count = src->count;
    dst->capacity = src->capacity;
    dst->item_size = src->item_size;
    dst->alloc_func = src->alloc_func;
    dst->free_func = src->free_func;

    src->data = NULL;
    src->count = -1;
    src->capacity = -1;
    src->item_size = -1;
    src->alloc_func = NULL;
    src->free_func = NULL;
}

int array_count(const struct Array* array)
{
    return array->count;
}

int array_capacity(struct Array* array)
{
    return array->capacity;
}

void array_add(struct Array* array, void* item)
{
    char* next_elem = _array_next_element(array);
    memcpy(next_elem, item, array->item_size);
    ++array->count;
}

void* array_emplace(struct Array* array, void* args)
{
    char* next_elem = _array_next_element(array);
    if(array->alloc_func)
    {
        array->alloc_func(next_elem, args);
    }
    ++array->count;
    return next_elem;
}

void array_remove_at(struct Array* array, int index)
{
    void* dst = array->data + (array->item_size * index);
    void* src = array->data + (array->item_size * (array->count-1));

    if(array->free_func)
    {
        array->free_func(dst);
    }

    memcpy(dst, src, array->item_size);
    --array->count;
}

void* array_get(const struct Array* array, int index)
{
    return array->data + (array->item_size * index);
}

void array_shrink(struct Array* array)
{
    _array_realloc(array, array->count);
}

int array_find(const struct Array* array, const void* item, compare_fn comp_func)
{
    for(int i = 0; i < array->count; ++i)
    {
        if(comp_func(array_get(array, i), item))
        {
            return i;
        }
    }

    return -1;
}

void array_sort(struct Array* array, compare_fn comp_func)
{
    qsort(array->data, array->count, array->item_size, comp_func);
}
