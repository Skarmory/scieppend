#ifndef SCIEPPEND_CORE_CONTAINER_COMMON_H
#define SCIEPPNED_CORE_CONTAINER_COMMON_H

typedef void(*alloc_fn)(void* item, const void* args);
typedef void(*free_fn)(void* item);
typedef int(*compare_fn)(const void* lhs, const void* rhs);

#endif
