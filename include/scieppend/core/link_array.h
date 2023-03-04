#ifndef SCIEPPEND_CORE_LINK_ARRAY_H
#define SCIEPPEND_CORE_LINK_ARRAY_H

struct LinkArrayNode
{
    int next;
    int prev;
    void*              data;
};

struct LinkArray
{
    int item_size;
    int count;
    int capacity;
    int usedhead;
    int freehead;
    struct LinkArrayNode* nodes;
};

struct LinkArray* linkarray_new(int item_size, int capacity);
void linkarray_free(struct LinkArray* list);
void linkarray_init(struct LinkArray* list, int item_size, int capacity);
void linkarray_uninit(struct LinkArray* list);

void  linkarray_add(struct LinkArray* list, void* item);
void* linkarray_pop_front(struct LinkArray* list);

#endif
