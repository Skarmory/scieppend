#ifndef SCIEPPEND_CORE_LIST_H
#define SCIEPPEND_CORE_LIST_H

#include <stdbool.h>

/* A non-intrusive doubly linked list.
 * Not the most optimal container, but it has some interesting functionality that would be more of a 
 * pain to implement with more memory-efficient implementations.
 */

typedef bool(*compare_func)(void* lhs, void* rhs);
typedef void(*dtor_func)(void* obj);

struct ListNode
{
    void* data;
    struct ListNode* next;
    struct ListNode* prev;
};

struct List
{
    int              count;
    struct ListNode* head;
    struct ListNode* tail;
};

/* Creates a new list on the heap and returns it.
 */
struct List* list_new(void);

/* Frees all the list nodes and the list itself.
 * Does NOT free the list node data.
 */
void list_free(struct List* list);

/* Frees all the list nodes and list node data, giving an empty list.
 * Will call the given destructor function, or else will call free() on the data.
 */
void list_free_data(struct List* list, dtor_func dtor);

/* Initialises the list.
 * Useful for stack allocated lists.
 */
void list_init(struct List* list);

/* Frees all the list nodes, giving an empty list.
 * Does NOT free the data the list nodes hold
 */
void list_uninit(struct List* list);

/* Add an item to the end of the list.
 */
void list_add(struct List* list, void* data);

/* Add node to the beginning of the list.
 */
void list_add_head(struct List* list, void* data);

/* Remove node from the list.
 */
void list_rm(struct List* list, struct ListNode* node);

/* Find an item based on pointer address.
 */
struct ListNode* list_find(struct List* list, void* data);

/* Removes a node from one list, and adds it into another list.
 */
void list_splice_node(struct List* list_from, struct List* list_to, struct ListNode* node);

/* Removes a range of nodes from one list, and inserts it into another list.
 */
void list_splice(struct List* list_from, struct List* list_to, int from_start, int to_start, int count);

/* Adds a data item into the list directly after the given node.
 */
void list_insert_after(struct List* list, void* insert_this, struct ListNode* after_this);

/* Adds a data item into the list directly before the given node.
 */
void list_insert_before(struct List* list, void* insert_this, struct ListNode* before_this);

/* Removes the data item from the beginning of the list and returns it.
 */
void* list_pop_head(struct List* list);

/* Traverses the list and removes the data item at the given index.
 */
void* list_pop_at(struct List* list, int index);

/* Returns the data item at the beginning of the list without removing it.
 */
void* list_peek_head(const struct List* list);

/* Returns the data item at the end of the list without removing it.
 */
void* list_peek_tail(const struct List* list);

/* Returns whether the list has any items.
 */
bool list_empty(const struct List* list);

/* Sorts the list using mergesort and the given data item compare function.
 */
void list_sort(struct List* list, compare_func func);

/* Check that the given node is a member of the given list.
 */
bool list_node_is_member(struct List* list, struct ListNode* node);

/* Helper macro for-each loop.
 */
#define list_for_each(list, it)\
    for(it = (list)->head; it != NULL; it = it->next)

/* Helper macro for-each loop where checks are performed to guard against null pointer dereference.
 * Use this if you want to modify the list whilst looping over it.
 */
#define list_for_each_safe(list, it, n)\
    for(it = (list)->head, n = (it ? it->next : NULL);\
        it != NULL;\
        it = n, n = (it ? it->next : NULL))

#endif
