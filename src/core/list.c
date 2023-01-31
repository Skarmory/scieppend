#include "core/list.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

// INTERNAL FUNCS

static inline struct ListNode* _list_node_new(void* data, struct ListNode* prev, struct ListNode* next)
{
    struct ListNode* node = malloc(sizeof(struct ListNode*));
    node->data = data;
    node->prev = prev;
    node->next = next;
    return node;
}

static inline void _add_node(struct List* list, struct ListNode* node)
{
    if(list->tail)
    {
        node->prev = list->tail;
        list->tail->next = node;
    }
    else
    {
        list->head = node;
    }

    ++list->count;
    list->tail = node;
}

static inline void _remove_node(struct List* list, struct ListNode* node)
{
    if(node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        list->head = node->next;
    }

    if(node->next)
    {
        node->next->prev = node->prev;
    }
    else
    {
        list->tail = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
    --list->count;
}

static struct ListNode* _node_at(struct List* list, int index)
{
    struct ListNode* node = list->head;

    while(index != 0)
    {
        node = node->next;
        --index;
    }

    return node;
}

static struct List _remove_range(struct List* from, int start, int count)
{
    struct ListNode* from_start = _node_at(from, start);
    struct ListNode* from_end = _node_at(from, start + count - 1);

    if(from_start->prev)
    {
        from_start->prev->next = from_end->next;
    }
    else
    {
        // from_start is head
        from->head = from_end->next;
    }

    if(from_end->next)
    {
        from_end->next->prev = from_start->prev;
    }
    else
    {
        // from_end is tail
        from->tail = from_start->prev;
    }

    from_start->prev = NULL;
    from_end->next   = NULL;

    from->count -= count;

    struct List ret;
    list_init(&ret);
    ret.head = from_start;
    ret.tail = from_end;
    ret.count = count;

    return ret;
}

static void _add_range(struct List* to, int start, int count, struct ListNode* range_start_node, struct ListNode* range_end_node)
{
    struct ListNode* to_start = _node_at(to, start);

    if(!to_start) // Empty list
    {
        to->head = range_start_node;
        to->tail = range_end_node;
    }
    else
    {
        // Link in the new range
        // This inserts the range at the "start" index
        range_start_node->prev = to_start->prev;
        range_end_node->next = to_start;

        if(to_start->prev)
        {
            to_start->prev->next = range_start_node;
        }
        else
        {
            to->head = range_start_node;
        }

        to_start->prev = range_end_node;
    }

    to->count += count;
}

/* Does a mergesort
 */
static void _sort(struct List* list, compare_func func)
{
    if(list->count <= 1)
    {
        return;
    }

    // Step 1: Split the top level list up into 2 sublists of equal(ish) size
    struct List left;
    struct List right;
    list_init(&left);
    list_init(&right);

    const int count = list->count;
    const int half_count = count/2;
    struct ListNode* node = list->head;
    struct ListNode* node_next = list->head->next;

    for(int i = 0; i < count; ++i)
    {
        if(i < half_count)
        {
            list_splice_node(list, &left, node);
        }
        else
        {
            list_splice_node(list, &right, node);
        }

        node = node_next;
        if(node_next)
        {
            node_next = node_next->next;
        }
    }

    // Step 2: Recurse the lists
    _sort(&left, func);
    _sort(&right, func);

    // Step 3: Compare the left and right list and splice them back onto the top level list
    struct ListNode* left_it = left.head;
    struct ListNode* right_it = right.head;
    while(left_it != NULL || right_it != NULL)
    {
        bool left_right = true;
        if(left_it && right_it)
        {
            // Both lists valid, so use compare function
            left_right = func(left_it->data, right_it->data);
        }
        else
        {
            // One of the lists is invalid, so check which is null to determine which to splice
            left_right = left_it != NULL;
        }

        if(left_right)
        {
            // Splict from left
            struct ListNode* n = left_it;
            left_it = left_it->next;
            list_splice_node(&left, list, n);
        }
        else
        {
            // Splice from right
            struct ListNode* n = right_it;
            right_it = right_it->next;
            list_splice_node(&right, list, n);
        }
    }
}

[[maybe_unused]] void _debug_check_membership([[maybe_unused]] struct List* list, [[maybe_unused]] struct ListNode* node)
{
#ifdef DEBUG_CORE_LIST
    assert(node != NULL && "List node is NULL");
    assert(list_node_is_member(list, node) && "List node is not a member of given list");
#endif
}

// EXTERNAL FUNCS

struct List* list_new(void)
{
    struct List* list = malloc(sizeof(struct List));
    list_init(list);
    return list;
}

void list_free(struct List* list)
{
    list_uninit(list);
    free(list);
}

void list_free_data(struct List* list, dtor_func dtor)
{
    struct ListNode *n = NULL, *nn = NULL;
    list_for_each_safe(list, n, nn)
    {
        if(dtor)
        {
            dtor(n->data);
        }
        else
        {
            free(n->data);
        }

        free(n);
    }

    list_init(list);
}

void list_init(struct List* list)
{
    list->count = 0;
    list->head = NULL;
    list->tail = NULL;
}

void list_uninit(struct List* list)
{
    struct ListNode *node, *n;
    list_for_each_safe(list, node, n)
    {
        free(node);
    }

    list->count = 0;
    list->head = NULL;
    list->tail = NULL;
}

void list_add(struct List* list, void* data)
{
    struct ListNode* node = _list_node_new(data, NULL, NULL);
    _add_node(list, node);
}

void list_add_head(struct List* list, void* data)
{
    struct ListNode* node = _list_node_new(data, NULL, NULL);

    if(list->head)
    {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    else
    {
        list->head = node;
        list->tail = node;
    }

    ++list->count;
}

void list_rm(struct List* list, struct ListNode* node)
{
    _debug_check_membership(list, node);
    _remove_node(list, node);
    free(node);
}

struct ListNode* list_find(struct List* list, void* data)
{
    struct ListNode* node = NULL;
    list_for_each(list, node)
    {
        if(node->data == data)
        {
            return node;
        }
    }

    return NULL;
}

void list_splice_node(struct List* list_from, struct List* list_to, struct ListNode* node)
{
    _debug_check_membership(list_from, node);
    _remove_node(list_from, node);
    _add_node(list_to, node);
}

void list_splice(struct List* list_from, struct List* list_to, int from_start, int to_start, int count)
{
#ifdef DEBUG_CORE_LIST
    assert(from_start + count < list_from->count && "Attempting to splice a range outside bounds of the given list");
    assert(to_start < list_to->count && "Attempting to splice range to an index outside bounds of the given list");
#endif

    if(count <= 0 || from_start < 0 || to_start < 0)
    {
        return;
    }

    struct List removed = _remove_range(list_from, from_start, count);
    _add_range(list_to, to_start, count, removed.head, removed.tail);
}

void list_insert_after(struct List* list, void* insert_this, struct ListNode* after_this)
{
    _debug_check_membership(list, after_this);

    struct ListNode* node = _list_node_new(insert_this, after_this, after_this->next);

    if(after_this->next)
    {
        after_this->next->prev = node;
    }
    else
    {
        list->tail = node;
    }

    after_this->next = node;
    ++list->count;
}

void list_insert_before(struct List* list, void* insert_this, struct ListNode* before_this)
{
    _debug_check_membership(list, before_this);

    struct ListNode* node = _list_node_new(insert_this, before_this->prev, before_this);

    if(before_this->prev)
    {
        before_this->prev->next = node;
    }
    else
    {
        list->head = node;
    }

    before_this->prev = node;
    ++list->count;
}

void* list_pop_head(struct List* list)
{
#ifdef DEBUG_CORE_LIST
    assert(list->count > 0);
#endif

    void* data = list->head->data;
    list_rm(list, list->head);
    return data;
}

void* list_pop_at(struct List* list, int index)
{
#ifdef DEBUG_CORE_LIST
    assert(index >= 0 && index < list->count);
#endif

    struct ListNode* node = list->head;
    while(index != 0)
    {
        node = node->next;
        --index;
    }

    void* data = node->data;
    list_rm(list, node);
    return data;
}

void* list_peek_head(const struct List* list)
{
    return list->head ? list->head->data : NULL;
}

void* list_peek_tail(const struct List* list)
{
    return list->tail ? list->tail->data : NULL;
}

bool list_empty(const struct List* list)
{
    return list->count == 0;
}

void list_sort(struct List* list, compare_func func)
{
    if(list->count == 0 || list->count == 1)
    {
        return;
    }

    _sort(list, func);
}

bool list_node_is_member(struct List* list, struct ListNode* node)
{
    struct ListNode* n = NULL;
    list_for_each(list, n)
    {
        if(node == n)
        {
            return true;
        }
    }

    return false;
}
