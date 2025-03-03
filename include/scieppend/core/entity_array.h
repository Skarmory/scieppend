#ifndef SCIEPPEND_CORE_ENTITY_ARRAY_H
#define SCIEPPEND_CORE_ENTITY_ARRAY_H

struct EntityArray;

struct EntityArray* entity_array_new(int capacity);
void entity_array_free(struct EntityArray* array);

#endif

