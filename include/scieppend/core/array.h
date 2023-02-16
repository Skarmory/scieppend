#ifndef SCIEPPEND_CORE_DYNAMIC_ARRAY_H
#define SCIEPPEND_CORE_DYNAMIC_ARRAY_H

#include <stdbool.h>

/* Dynamically sized array.
 */

typedef void(*free_function)(void*);
typedef bool(*comp_function)(void* lhs, void* rhs);

struct Array;

struct Array* array_new(int item_size, int capacity, free_function free_func);
void          array_free(struct Array* array);
int           array_count(struct Array* array);
int           array_capacity(struct Array* array);
void          array_add(struct Array* array, void* item);
void          array_remove(struct Array* array, int index);
void*         array_get(struct Array* array, int index);
void          array_shrink(struct Array* array);
int           array_find(struct Array*, void* item, comp_function comp_func);

#endif
