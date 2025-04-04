#ifndef SCIEPPEND_CORE_CACHE_H
#define SCIEPPEND_CORE_CACHE_H

#include "scieppend/core/container_common.h"

#include <stdbool.h>

/* Data container that stores items in contiguous memory.
 * Maintains a free list for quick allocation.
 * Handles embed the generation key and the storage index.
 *
 * The allocation process is:
 *      1) Check free list, if not empty return the first handle from it.
 *      2) Check whether we can unlock a new handle, return it if we can.
 *      3) We have no free list items left and no capacity left, so return a null handle.
 *
 * NOTE: Re-using free list handles increases the key generation by 1. It is possible for a rollover
 *       to happen after reusing the same handle 32767 times.
 *
 * NOTE: It is up to the user to figure out how to handle capacity issues and key rollover issues.
 *
 * NOTE: It is up to the user to use handles correctly. There is no way of knowing that handles
 *       match the cache which they are passed to.
 */

extern const int C_NULL_CACHE_HANDLE;

struct Cache
{
    void* items;
    int*  handles;
    int   current_used;
    int   max_used;
    int   item_size;
    int   capacity;
    int   free_head;
    alloc_fn alloc_func;
    free_fn  free_func;
};

struct CacheIt
{
    struct Cache* cache;
    int           current_idx;
};

/* Create a new cache with given capacity and optional destructor for elements.
 */
struct Cache* cache_new(int item_size, int capacity, alloc_fn alloc_func, free_fn free_func);

/* Constructs a new cache in-place.
 */
void cache_init(struct Cache* cache, int item_size, int capacity, alloc_fn alloc_func, free_fn free_func);

/* Frees all the elements, calling the destructor if specified, then frees the cache.
 */
void cache_free(struct Cache* cache);

/* Frees cache internals, does not free the given pointer.
 */
void cache_uninit(struct Cache* cache);

/* Returns the number of currently valid handles.
 * This excludes the handles that have been unlocked but are currently invalid.
 */
int cache_size(const struct Cache* cache);

/* Return the maximum possible number of elements the cache can hold.
 */
int cache_capacity(const struct Cache* cache);

/* Returns the size of a data element in bytes.
 */
int cache_item_size(const struct Cache* cache);

/* Returns the number of unlocked handles.
 * This is the number of both valid and invalid handles.
 */
int cache_used(const struct Cache* cache);

/* Check if the key generation for a handle matches what the cache is holding.
 * If true, the slot has been freed and reused.
 */ 
bool cache_stale_handle(const struct Cache* cache, int handle);

/* Adds an element to the cache and returns a handle to access it.
 * See allocation process at top of this file for further info.
 * Copies the item.
 */
int cache_add(struct Cache* cache, const void* item);

/* Constructs an item with the given args and returns a handle to it.
 */
int cache_emplace(struct Cache* cache, const void* args);

/* Removes an element from the cache.
 */
void cache_remove(struct Cache* cache, int handle);

/* Returns the element a handle points to.
 */
void* cache_get(const struct Cache* cache, int handle);

// CACHE ITERATOR

/* Make an iterator to the first item in the cache.
 */
struct CacheIt cache_begin(struct Cache* cache);

/* Make an interator for the past the elements in the cache.
 */
struct CacheIt cache_end(struct Cache* cache);

/* Get the next element in the cache.
 */
struct CacheIt cache_it_next(struct CacheIt it);

/* Check if two iterators are equal.
 */
bool cache_it_eq(struct CacheIt lhs, struct CacheIt rhs);

/* Get value from the cache iterator.
 */
void* cache_it_get(struct CacheIt it);

#endif
