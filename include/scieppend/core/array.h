#ifndef SCIEPPEND_CORE_DYNAMIC_ARRAY_H
#define SCIEPPEND_CORE_DYNAMIC_ARRAY_H

#include <stdbool.h>

/* Dynamically sized array.
 */

typedef void(*array_alloc_function)(void* item, void* args);
typedef void(*array_free_function)(void* item);
typedef bool(*array_comp_function)(void* lhs, void* rhs);

struct Array
{
    char*         data;
    int           count;
    int           capacity;
    int           item_size;
    array_alloc_function alloc_func;
    array_free_function  free_func;
};

struct Array* array_new(int item_size, int capacity, array_alloc_function alloc_func, array_free_function free_func);
void          array_init(struct Array* array, int item_size, int capacity, array_alloc_function alloc_func, array_free_function free_func);
void          array_free(struct Array* array);
void          array_uninit(struct Array* array);
int           array_count(struct Array* array);
int           array_capacity(struct Array* array);
void          array_add(struct Array* array, void* item);
void*         array_emplace(struct Array* array, void* args);
void          array_remove_at(struct Array* array, int index);
void*         array_get(struct Array* array, int index);
void          array_shrink(struct Array* array);
int           array_find(struct Array*, void* item, array_comp_function comp_func);

#endif
