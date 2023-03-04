#include "scieppend/core/link_array.h"

#include <stdlib.h>
#include <string.h>

static void _grow_array(struct LinkArray* array)
{
    int new_cap = array->capacity << 1;
    int per_node_size = 2 * sizeof(struct LinkArrayNode*) + array->item_size;
    array->nodes = realloc(array->nodes, per_node_size * new_cap);

    if(!array->nodes)
    {
        // Something has gone horribly wrong
        abort();
    }

    array->capacity = new_cap;
}

static void _unlink_node(struct LinkArray* array, struct LinkArrayNode* node)
{
    if(node->prev != -1)
    {
        struct LinkArrayNode* prevnode = &array->nodes[node->prev];
        prevnode->next = node->next;
    }

    if(node->next != -1)
    {
        struct LinkArrayNode* nextnode = &array->nodes[node->next];
        nextnode->prev = node->prev;
    }

    node->prev = -1;
    node->next = -1;
}

struct LinkArray* linkarray_new(int item_size, int capacity)
{
    struct LinkArray* array = malloc(sizeof(struct LinkArray));
    linkarray_init(array, item_size, capacity);
    return array;
}

void linkarray_free(struct LinkArray* array)
{
    linkarray_uninit(array);
    free(array);
}

void linkarray_init(struct LinkArray* array, int item_size, int capacity)
{
    array->item_size = item_size;
    array->count = 0;
    array->capacity = capacity > 0 ? capacity : 8;

    int per_node_size = 2 * sizeof(int) + array->item_size;
    array->nodes = malloc(per_node_size * array->capacity);

    char* node = (char*)array->nodes;
    for(int i = 0; i < array->capacity; ++i)
    {
        struct LinkArrayNode* arraynode = (struct LinkArrayNode*)node;
        arraynode->next = (i + 1) < array->capacity ? i + 1 : -1;
        arraynode->prev = (i - 1) >= 0 ? i - 1 : -1;
        node += per_node_size;
    }

    array->usedhead = -1;
    array->freehead = 0;
}

void linkarray_uninit(struct LinkArray* array)
{
    free(array->nodes);
}

void linkarray_add(struct LinkArray* array, void* item)
{
    if(array->count == array->capacity)
    {
        _grow_array(array);
    }

    int next_item_idx = array->freehead;
    struct LinkArrayNode* next_item = &array->nodes[next_item_idx];

    // Remove from freearray
    array->freehead = array->nodes[next_item_idx].next;
    _unlink_node(array, next_item);

    // Add to usedarray
    next_item->next = array->usedhead;
    if(array->usedhead != -1)
    {
        array->nodes[array->usedhead].prev = next_item_idx;
    }
    array->usedhead = next_item_idx;

    memcpy(&next_item->data, item, array->item_size);

    ++array->count;
}

void* linkarray_pop_front(struct LinkArray* array)
{
    int pop_idx = array->usedhead;
    struct LinkArrayNode* pop_this = &array->nodes[pop_idx];

    array->usedhead = pop_this->next;
    _unlink_node(array, pop_this);

    pop_this->next = array->freehead;
    if(array->freehead != -1)
    {
        struct LinkArrayNode* freehead = &array->nodes[array->freehead];
        freehead->prev = pop_idx;
    }
    array->freehead = pop_idx;

    --array->count;

    return pop_this->data;
}
