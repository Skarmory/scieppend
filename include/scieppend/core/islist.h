#ifndef SCIEPPEND_CORE_ISLIST_H
#define SCIEPPEND_CORE_ISLIST_H

struct ISListNode
{
    struct ISListNode* next;
    struct ISListNode* prev;
    void*              data;
};

struct ISList
{
    int item_size;
    int count;
    int capacity;
    struct ISListNode* usedhead;
    struct ISListNode* freehead;
    struct ISListNode* nodes;
};

struct ISList* islist_new(int item_size, int capacity);
void islist_free(struct ISList* list);
void islist_init(struct ISList* list, int item_size, int capacity);
void islist_uninit(struct ISList* list);

void islist_add(struct ISList* list, void* item);
void* islist_pop_front(struct ISList* list);

#endif
