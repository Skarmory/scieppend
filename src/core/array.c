#include "scieppend/core/array.h"

#include <stdlib.h>
#include <string.h>

static const int C_DEFAULT_CAPACITY = 8;

static void _array_default_alloc_func([[maybe_unused]] void* item, [[maybe_unused]] void* args)
{
}

static void _array_default_free_func([[maybe_unused]] void* item)
{
}

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

    return &array->data[(array->item_size * array->count)];
}

struct Array* array_new(int item_size, int capacity, array_alloc_function alloc_func, array_free_function free_func)
{
    struct Array* array = malloc(sizeof(struct Array));
    array_init(array, item_size, capacity, alloc_func, free_func);
    return array;
}

void array_init(struct Array* array, int item_size, int capacity, array_alloc_function alloc_func, array_free_function free_func)
{
    array->capacity   = capacity > 0 ? capacity : C_DEFAULT_CAPACITY;
    array->count      = 0;
    array->item_size  = item_size;
    array->data       = malloc(item_size * array->capacity);
    array->alloc_func = alloc_func ? alloc_func : &_array_default_alloc_func;
    array->free_func  = free_func ? free_func : &_array_default_free_func;
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

int array_count(struct Array* array)
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
    array->alloc_func(next_elem, args);
    ++array->count;
    return next_elem;
}

void array_remove_at(struct Array* array, int index)
{
    if(array->free_func)
    {
        array->free_func(&array->data[array->item_size * index]);
    }

    memcpy(&array->data[array->item_size * index], &array->data[array->item_size * array->count-1], array->item_size);
    memset(&array->data[array->item_size * array->count-1], '\0', array->item_size);
    --array->count;
}

void* array_get(struct Array* array, int index)
{
    return &array->data[array->item_size * index];
}

void array_shrink(struct Array* array)
{
    _array_realloc(array, array->count);
}

int array_find(struct Array* array, void* item, array_comp_function comp_func)
{
    for(int i = 0; i < array->count; ++i)
    {
        if(comp_func(&array->data[array->item_size * i], item))
        {
            return i;
        }
    }

    return -1;
}
