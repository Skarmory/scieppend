#include "scieppend/core/link_array.h"

#include <stdlib.h>
#include <string.h>

static inline int _node_bytes(struct LinkArray* array)
{
    return (2 * sizeof(int)) + array->item_size;
}

static inline int _get_offset_bytes(struct LinkArray* array, int idx)
{
    return idx * _node_bytes(array);
}

static inline struct LinkArrayNode* _get_node(struct LinkArray* array, int idx)
{
    return (struct LinkArrayNode*) &array->nodes[_get_offset_bytes(array, idx)];
}

static void _link_nodes(struct LinkArray* array, int start)
{
    for(int i = start; i < array->capacity; ++i)
    {
        struct LinkArrayNode* arraynode = _get_node(array, i);
        arraynode->next = (i + 1) <  array->capacity ? i + 1 : -1;
        arraynode->prev = (i - 1) >= array->count    ? i - 1 : -1;
    }

    array->freehead = array->count;
}

static void _grow_array(struct LinkArray* array)
{
    int new_cap = array->capacity << 1;
    array->nodes = realloc(array->nodes, _node_bytes(array) * new_cap);

    if(!array->nodes)
    {
        // Something has gone horribly wrong
        abort();
    }

    array->capacity = new_cap;
    _link_nodes(array, array->count);
}

static void _make_free_node(struct LinkArray* array, struct LinkArrayNode* node, int nidx)
{
    // Remove from usedlist
    if(node->prev != -1)
    {
        // Not head
        struct LinkArrayNode* prevnode = _get_node(array, node->prev);
        prevnode->next = node->next;
    }
    else
    {
        // Is head
        array->usedhead = node->next;
    }

    if(node->next != -1)
    {
        struct LinkArrayNode* nextnode = _get_node(array, node->next);
        nextnode->prev = node->prev;
    }

    // Add to freelist
    if(array->freehead != -1)
    {
        // Non-empty freelist
        struct LinkArrayNode* freehead = _get_node(array, array->freehead);
        freehead->prev = nidx;
    }

    node->next = array->freehead;
    node->prev = -1;
    array->freehead = nidx;
}

static void _make_used_node(struct LinkArray* array, struct LinkArrayNode* node, int nidx)
{
    // Remove from freelist
    if(node->prev != -1)
    {
        // Not head
        struct LinkArrayNode* prevnode = _get_node(array, node->prev);
        prevnode->next = node->next;
    }
    else
    {
        // Is head
        array->freehead = node->next;
    }

    if(node->next != -1)
    {
        struct LinkArrayNode* nextnode = _get_node(array, node->next);
        nextnode->prev = node->prev;
    }

    // Add to used list
    if(array->usedhead != -1)
    {
        // Non-empty usedlist
        struct LinkArrayNode* usedhead = _get_node(array, array->usedhead);
        usedhead->prev = nidx;
    }

    node->next = array->usedhead;
    node->prev = -1;
    array->usedhead = nidx;
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

    _link_nodes(array, 0);

    array->usedhead = -1;
    array->freehead = 0;
}

void linkarray_uninit(struct LinkArray* array)
{
    array->item_size = 0;
    array->count = 0;
    array->capacity = 0;
    array->usedhead = -1;
    array->freehead = -1;
    free(array->nodes);
}

void linkarray_add(struct LinkArray* array, void* item)
{
    if(array->count == array->capacity)
    {
        _grow_array(array);
        _link_nodes(array, array->count);
    }

    int next_item_idx = array->freehead;
    struct LinkArrayNode* next_item = _get_node(array, next_item_idx);

    _make_used_node(array, next_item, next_item_idx);

    memcpy((char*)next_item + (2 * sizeof(int)), item, array->item_size);

    ++array->count;
}

char** linkarray_pop_front_voidp(struct LinkArray* array)
{
    int pop_idx = array->usedhead;
    struct LinkArrayNode* pop_this = _get_node(array, pop_idx);

    _make_free_node(array, pop_this, pop_idx);

    --array->count;

    return &pop_this->data;
}
