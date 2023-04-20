#include "scieppend/core/link_array.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static inline int _node_bytes(struct LinkArray* array)
{
    return offsetof(struct LinkArrayNode, data) + array->item_size;
}

static inline int _get_offset_bytes(struct LinkArray* array, int idx)
{
    return idx * _node_bytes(array);
}

static inline void* _get_node_data(struct LinkArrayNode* node)
{
    return (char*)node + offsetof(struct LinkArrayNode, data);
}

static inline struct LinkArrayNode* _get_node(struct LinkArray* array, int idx)
{
    if(idx < 0 || idx >= array->capacity)
    {
        return NULL;
    }
    return (struct LinkArrayNode*) &((char*)array->nodes)[_get_offset_bytes(array, idx)];
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

    if(node->next != -1)
    {
        // Not tail
        struct LinkArrayNode* nextnode = _get_node(array, node->next);
        nextnode->prev = node->prev;
    }
    else
    {
        // Is tail
        array->usedtail = node->prev;
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

static void _make_used_node(struct LinkArray* array, struct LinkArrayNode* node, int nidx, bool head)
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

    if(head)
    {
        // Add to used list at list head
        if(array->usedhead != -1)
        {
            // Non-empty usedlist
            struct LinkArrayNode* usedhead = _get_node(array, array->usedhead);
            usedhead->prev = nidx;
        }
        else
        {
            // Empty usedlist
            array->usedtail = nidx;
        }

        node->prev = -1;
        node->next = array->usedhead;
        array->usedhead = nidx;
    }
    else
    {
        // Add to used list at list tail
        if(array->usedtail != -1)
        {
            // Non-empty usedlist
            struct LinkArrayNode* usedtail = _get_node(array, array->usedtail);
            usedtail->next = nidx;
        }
        else
        {
            // Empty usedlist
            array->usedhead = nidx;
        }

        node->next = -1;
        node->prev = array->usedtail;
        array->usedtail = nidx;
    }
}

static void _internal_pop(struct LinkArray* array, int head_or_tail)
{
    struct LinkArrayNode* pop_this = _get_node(array, head_or_tail);

    if(pop_this)
    {
        _make_free_node(array, pop_this, head_or_tail);
        --array->count;
        if(array->free)
        {
            array->free(_get_node_data(pop_this));
        }
    }
}

static struct LinkArrayNode* _internal_get_next_node(struct LinkArray* array, bool head)
{
    if(array->count == array->capacity)
    {
        _grow_array(array);
        _link_nodes(array, array->count);
    }

    int next_item_idx = array->freehead;
    struct LinkArrayNode* next_item = _get_node(array, next_item_idx);
    _make_used_node(array, next_item, next_item_idx, head);

    return next_item;
}

static void _internal_add_by_copy(struct LinkArray* array, void* item, bool head)
{
    struct LinkArrayNode* next_item = _internal_get_next_node(array, head);
    memcpy(_get_node_data(next_item), item, array->item_size);
    ++array->count;
}

static void _internal_add_by_alloc_func(struct LinkArray* array, void* args, bool head)
{
    struct LinkArrayNode* next_item = _internal_get_next_node(array, head);
    array->alloc(_get_node_data(next_item), args);
    ++array->count;
}

static struct LinkArrayNode* _internal_at(struct LinkArray* array, int index)
{
    if(array->usedhead == -1 || index < 0 || index >= array->count)
    {
        return NULL;
    }

    struct LinkArrayNode* node =  NULL;

    if((array->count - index) >= (array->count / 2))
    {
        // Forward iterate
        node = _get_node(array, array->usedhead);
        while(index != 0)
        {
            node = _get_node(array, node->next);
            --index;
        }
    }
    else
    {
        // Reverse iterate
        node = _get_node(array, array->usedtail);
        while(index != array->count - 1)
        {
            node = _get_node(array, node->prev);
            ++index;
        }
    }

    return node;
}

struct LinkArray* linkarray_new(int item_size, int capacity, alloc_fn alloc, free_fn free)
{
    struct LinkArray* array = malloc(sizeof(struct LinkArray));
    linkarray_init(array, item_size, capacity, alloc, free);
    return array;
}

void linkarray_free(struct LinkArray* array)
{
    linkarray_uninit(array);
    free(array);
}

void linkarray_init(struct LinkArray* array, int item_size, int capacity, alloc_fn alloc, free_fn free)
{
    array->item_size = item_size;
    array->count = 0;
    array->capacity = capacity > 0 ? capacity : 8;
    array->usedhead = -1;
    array->usedtail = -1;
    array->freehead = 0;
    array->alloc = alloc;
    array->free  = free;

    array->nodes = malloc(_node_bytes(array) * array->capacity);

    _link_nodes(array, 0);
}

void linkarray_uninit(struct LinkArray* array)
{
    array->item_size = 0;
    array->count     = 0;
    array->capacity  = 0;
    array->usedhead  = -1;
    array->usedtail  = -1;
    array->freehead  = -1;
    array->alloc     = NULL;
    array->free      = NULL;
    free(array->nodes);
}

int linkarray_count(struct LinkArray* array)
{
    return array->count;
}

void linkarray_push_front(struct LinkArray* array, void* item)
{
    _internal_add_by_copy(array, item, true);
}

void linkarray_push_back(struct LinkArray* array, void* item)
{
    _internal_add_by_copy(array, item, false);
}

void linkarray_clear(struct LinkArray* array)
{
    while(array->count > 0)
    {
        linkarray_pop_front(array);
    }
}

void* linkarray_front_voidp(struct LinkArray* array)
{
    int peek_idx = array->usedhead;
    struct LinkArrayNode* peek_this = _get_node(array, peek_idx);

    if(peek_this)
    {
        return _get_node_data(peek_this);
    }

    return NULL;
}

void* linkarray_back_voidp(struct LinkArray* array)
{
    int peek_idx = array->usedtail;
    struct LinkArrayNode* peek_this = _get_node(array, peek_idx);

    if(peek_this)
    {
        return _get_node_data(peek_this);
    }

    return NULL;
}

void* linkarray_at_voidp(struct LinkArray* array, int index)
{
    struct LinkArrayNode* node = _internal_at(array, index);
    return _get_node_data(node);
}

void linkarray_pop_front(struct LinkArray* array)
{
    _internal_pop(array, array->usedhead);
}

void linkarray_pop_back(struct LinkArray* array)
{
    _internal_pop(array, array->usedtail);
}

void linkarray_pop_at(struct LinkArray* array, int index)
{
    struct LinkArrayNode* pop_this = _internal_at(array, index);

    if(pop_this)
    {
        _make_free_node(array, pop_this, index);
        --array->count;
        if(array->free)
        {
            array->free(_get_node_data(pop_this));
        }
    }
}

void linkarray_emplace_back(struct LinkArray* array, void* args)
{
    _internal_add_by_alloc_func(array, args, false);
}

struct LinkArrayIt linkarray_begin(struct LinkArray* array)
{
    struct LinkArrayIt it;
    it.array = array;
    it.index = array->usedhead;
    return it;
}

struct LinkArrayIt linkarray_end(struct LinkArray* array)
{
    struct LinkArrayIt it;
    it.array = array;
    it.index = -1;
    return it;
}

bool linkarray_it_eq(struct LinkArrayIt lhs, struct LinkArrayIt rhs)
{
    return (lhs.array == rhs.array && lhs.index == rhs.index);
}

struct LinkArrayIt linkarray_it_next(struct LinkArrayIt it)
{
    struct LinkArrayNode* node = _get_node(it.array, it.index);
    struct LinkArrayIt next = it;
    next.index = node->next;
    return next;
}

void* linkarray_it_get_voidp(struct LinkArrayIt it)
{
    struct LinkArrayNode* node = _get_node(it.array, it.index);
    return _get_node_data(node);
}
