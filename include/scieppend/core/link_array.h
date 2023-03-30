#ifndef SCIEPPEND_CORE_LINK_ARRAY_H
#define SCIEPPEND_CORE_LINK_ARRAY_H

struct LinkArrayNode
{
    int   next;
    int   prev;
    void* data;
};

struct LinkArray
{
    int item_size;
    int count;
    int capacity;
    int usedhead;
    int freehead;
    void* nodes;
};

struct LinkArrayIt
{
    struct LinkArray* array;
    int               index;
};

struct LinkArray* linkarray_new(int item_size, int capacity);
void linkarray_free(struct LinkArray* array);
void linkarray_init(struct LinkArray* array, int item_size, int capacity);
void linkarray_uninit(struct LinkArray* array);

void linkarray_add(struct LinkArray* array, void* item);
#define linkarray_pop_front(array, type) ((type) *((type*)linkarray_pop_front_voidp((array))))
char** linkarray_pop_front_voidp(struct LinkArray* array);

// Iterator functions

struct LinkArrayIt linkarray_begin(struct LinkArray* array);
struct LinkArrayIt linkarray_end(struct LinkArray* array);
bool               linkarray_it_eq(struct LinkArrayIt lhs, struct LinkArrayIt rhs);
struct LinkArrayIt linkarray_it_next(struct LinkArrayIt it);
void**             linkarray_it_get_voidp(struct LinkArrayIt it);

#define linkarray_it_get(it, type) ((type) *((type*)linkarray_it_get_voidp((it))))

#endif
