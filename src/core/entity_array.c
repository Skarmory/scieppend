#include "scieppend/core/entity_array.h"

#include "scieppend/core/array_threadsafe.h"

#include <stdlib.h>

struct EntityArray
{
    struct Array_ThreadSafe entities;
};

struct EntityArray* entity_array_new(int capacity)
{
    struct EntityArray* new_array = malloc(sizeof(struct EntityArray));
    return new_array;
}

void entity_array_free(struct EntityArray* array)
{
    free(array);
}

