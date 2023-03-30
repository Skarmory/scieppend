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
    char* nodes;
};

struct LinkArray* linkarray_new(int item_size, int capacity);
void linkarray_free(struct LinkArray* array);
void linkarray_init(struct LinkArray* array, int item_size, int capacity);
void linkarray_uninit(struct LinkArray* array);

void linkarray_add(struct LinkArray* array, void* item);
#define linkarray_pop_front(array, type) ((type) *((type*)linkarray_pop_front_voidp((array))))
char** linkarray_pop_front_voidp(struct LinkArray* array);

#endif
