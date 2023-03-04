#include "scieppend/core/islist.h"

#include <stdlib.h>
#include <string.h>

static void _unlink_node(struct ISListNode* node)
{
    if(node->prev)
    {
        node->prev->next = node->next;
    }

    if(node->next)
    {
        node->next->prev = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
}

struct ISList* islist_new(int item_size, int capacity)
{
    struct ISList* list = malloc(sizeof(struct ISList));
    islist_init(list, item_size, capacity);
    return list;
}

void islist_free(struct ISList* list)
{
    islist_uninit(list);
    free(list);
}

void islist_init(struct ISList* list, int item_size, int capacity)
{
    list->item_size = item_size;
    list->count = 0;
    list->capacity = capacity > 0 ? capacity : 8;

    int per_node_size = sizeof(struct ISListNode*) * 2 + list->item_size;
    list->nodes = malloc(per_node_size * list->capacity);

    char* node = (char*)list->nodes;
    for(int i = 0; i < list->capacity; ++i)
    {
        struct ISListNode* listnode = (struct ISListNode*)node;
        listnode->next = (i + 1) < list->capacity ? (struct ISListNode*)(node + per_node_size) : NULL;
        listnode->prev = (i - 1) >= 0 ? (struct ISListNode*)(node - per_node_size) : NULL;
        node += per_node_size;
    }

    list->usedhead = NULL;
    list->freehead = list->nodes;
}

void islist_uninit(struct ISList* list)
{
    free(list->nodes);
}

void islist_add(struct ISList* list, void* item)
{
    struct ISListNode* next_item = list->freehead;

    // Remove from freelist
    list->freehead = list->freehead->next;
    _unlink_node(next_item);

    // Add to usedlist
    next_item->next      = list->usedhead;
    if(list->usedhead)
    {
        list->usedhead->prev = next_item;
    }
    list->usedhead       = next_item;

    memcpy(&next_item->data, item, list->item_size);

    ++list->count;
}

void* islist_pop_front(struct ISList* list)
{
    struct ISListNode* pop_this = list->usedhead;

    list->usedhead = list->usedhead->next;
    _unlink_node(pop_this);

    pop_this->next = list->freehead;
    if(list->freehead)
    {
        list->freehead->prev = pop_this;
    }
    list->freehead = pop_this;

    --list->count;

    return pop_this->data;
}
