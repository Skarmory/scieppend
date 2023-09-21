#include "scieppend/core/cache.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

// CONST

static const int C_IDX_MASK        = 0b00000000000000001111111111111111; // <<  0
static const int C_KEY_MASK        = 0b01111111111111110000000000000000; // << 16
static const int C_INVALID_MASK    = 0b10000000000000000000000000000000; // << 31
static const int C_VALID_MASK      = ~C_INVALID_MASK;
static const int C_KEY_SHIFT       = 16;
static const int C_MAX_GENERATIONS = C_KEY_MASK >> C_KEY_SHIFT;
static const int C_MAX_CAPACITY    = C_IDX_MASK - 1;

const int C_NULL_CACHE_HANDLE = 0xffffffff;

// INTERNAL FUNCS

// Bit logic to make our key and index into a handle
static int _make_handle(int key, int idx)
{
    // 0x00kkkkkkiiiiiiii
    return ( key << C_KEY_SHIFT ) | idx;
}

// Bit logic to get the item array index from the handle
static int _get_idx(int handle)
{
    return handle & C_IDX_MASK;
}

// Bit logic to get the key generation from the handle
static int _get_key(int handle)
{
    int key = handle & C_KEY_MASK;
    key >>= C_KEY_SHIFT;
    return key;
}

// Return the bytes offset from the beginning of cache items array
static int _get_item_offset(int item_size, int handle)
{
    return item_size * _get_idx(handle);
}

// Returns pointer to the item in the cache items array
static void* _get_item(struct Cache* cache, int handle)
{
    return (char*)cache->items + _get_item_offset(cache->item_size, handle);
}

// Check whether the free list has items.
static bool _free_list_empty(struct Cache* cache)
{
    return (_get_idx(cache->free_head) & C_IDX_MASK) == C_IDX_MASK;
}

/* Get the next handle index, either from the free list or open up a new entry.
 * If retrieving from the free list, make sure the free list is maintained.
 * Return invalid handle if there is nothing available.
 */
static int _next_handle(struct Cache* cache)
{
    int handle = C_NULL_CACHE_HANDLE;
    int key = C_KEY_MASK;
    int idx = C_IDX_MASK;

    if(!_free_list_empty(cache))
    {
        // Get from free list
        idx = _get_idx(cache->free_head);
        cache->handles[idx] &= C_VALID_MASK;

        key = _get_key(cache->handles[idx]);
        int next_idx = _get_idx(cache->handles[idx]);

        ++key;
        if(key == C_MAX_GENERATIONS)
        {
#ifdef DEBUG_CORE_CACHE
            printf("WARNING: KEY ROLLOVER AT INDEX: %d\n", idx);
#endif
            key = 0;
        }

        // Make return handle
        handle = _make_handle(key, idx);

        // Make entry valid
        cache->handles[idx] = handle;

        // Set free head to next node along
        cache->free_head = _make_handle(0, next_idx);
        if(next_idx == C_IDX_MASK)
        {
            cache->free_head |= C_INVALID_MASK;
        }
    }
    else
    {
        if(cache->max_used == cache->capacity)
        {
            return handle;
        }

        // No free list, open up new entry
        key = 0;
        idx = cache->max_used;

        cache->handles[idx] = _make_handle(key, idx);
        cache->handles[idx] &= C_VALID_MASK;

        handle = cache->handles[idx];

        ++cache->max_used;
    }

    return handle;
}

// Check if the handle is marked "invalid", therefore is a free slot
static bool _check_valid(int handle)
{
    return (handle & C_INVALID_MASK) == 0;
}

// Checks whether the handle passed in by other code matches the handle the cache is expecting
static bool _check_handle(struct Cache* cache, int handle)
{
    int idx = _get_idx(handle);

    if(idx >= cache->max_used)
    {
        return false;
    }

    if(!_check_valid(cache->handles[idx]))
    {
        return false;
    }

    if(cache_stale_handle(cache, handle))
    {
        return false;
    }

    return true;
}

[[maybe_unused]] static void _debug_print_free_list([[maybe_unused]] struct Cache* cache)
{
#ifdef DEBUG_CORE_CACHE
    int idx = _get_idx(cache->free_head);

    do
    {
        printf("%d > ", idx);
        idx = _get_idx(cache->handles[idx]);
    }
    while(idx != C_IDX_MASK);

    printf("%d\n ", C_IDX_MASK);
#endif
}

/* Print a handle out as an (index, key) pair
 * Invalid keys marked with '*'
 */ 
[[maybe_unused]] static void _debug_print_handle([[maybe_unused]] int handle)
{
#ifdef DEBUG_CORE_CACHE
    if(!_check_valid(handle))
    {
        printf("*");
    }

    printf("(i:%d, k:%d)", _get_idx(handle), _get_key(handle));
#endif
}

[[maybe_unused]] static void _debug_print_handles([[maybe_unused]] struct Cache* cache)
{
#ifdef DEBUG_CORE_CACHE
    _debug_print_handle(cache->handles[0]);

    for(int i = 1; i < cache->max_used; ++i)
    {
        printf(", ");
        _debug_print_handle(cache->handles[i]);
    }

    printf("\n");
#endif
}

static void _null_alloc([[maybe_unused]] void* item, [[maybe_unused]] void* args)
{
}

static void _null_free([[maybe_unused]] void* item)
{
}

// EXTERNAL FUNCS

struct Cache* cache_new(int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    assert(capacity <= C_MAX_CAPACITY);

    struct Cache* cache = malloc(sizeof(struct Cache));
    cache_init(cache, item_size, capacity, alloc_func, free_func);

    return cache;
}

void cache_init(struct Cache* cache, int item_size, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    cache->items        = malloc(capacity * item_size);
    cache->handles      = malloc(sizeof(int) * capacity);
    cache->current_used = 0;
    cache->max_used     = 0;
    cache->item_size    = item_size;
    cache->capacity     = capacity;
    cache->free_head    = C_NULL_CACHE_HANDLE;
    cache->alloc_func   = alloc_func ? alloc_func : &_null_alloc;
    cache->free_func    = free_func ? free_func : &_null_free;

    for(int i = 0; i < cache->capacity; ++i)
    {
        cache->handles[i] = C_NULL_CACHE_HANDLE;
    }
}

void cache_free(struct Cache* cache)
{
    cache_uninit(cache);
    free(cache);
}

void cache_uninit(struct Cache* cache)
{
    if(cache->free_func)
    {
        for(int i = 0; i < cache->max_used; ++i)
        {
            int handle = cache->handles[i];
            if(_check_valid(handle))
            {
                cache->free_func(_get_item(cache, handle));
            }
        }
    }

    free(cache->items);
    free(cache->handles);
}

int cache_size(struct Cache* cache)
{
    return cache->current_used;
}

int cache_capacity(struct Cache* cache)
{
    return cache->capacity;
}

int cache_item_size(struct Cache* cache)
{
    return cache->item_size;
}

int cache_used(struct Cache* cache)
{
    return cache->max_used;
}

bool cache_stale_handle(struct Cache* cache, int handle)
{
    return _get_key(handle) != _get_key(cache->handles[_get_idx(handle)]);
}

int cache_add(struct Cache* cache, const void* item)
{
    int next_handle = _next_handle(cache);

    if(_check_valid(next_handle))
    {
        memcpy(_get_item(cache, next_handle), item, cache->item_size);
        ++cache->current_used;
    }

    return next_handle;
}

int cache_emplace(struct Cache* cache, void* args)
{
    int next_handle = _next_handle(cache);

    if(_check_valid(next_handle))
    {
        void* new_item = _get_item(cache, next_handle);
        cache->alloc_func(new_item, args);
        ++cache->current_used;
    }

    return next_handle;
}

void cache_remove(struct Cache* cache, int handle)
{
    if(!_check_handle(cache, handle))
    {
        return;
    }

    int handle_idx = _get_idx(handle);

    cache->free_func(_get_item(cache, handle));

    if(!_free_list_empty(cache))
    {
        // Set invalidated handle to point at what free head is pointing at
        cache->handles[handle_idx] = _make_handle(_get_key(cache->handles[handle_idx]), _get_idx(cache->free_head));
        cache->handles[handle_idx] |= C_INVALID_MASK;

        // Set free head to point at invalidated handle
        cache->free_head = _make_handle(0, handle_idx);
    }
    else
    {
        // Set invalidated handle to point at index mask
        cache->handles[handle_idx] = _make_handle(_get_key(cache->handles[handle_idx]), C_IDX_MASK);
        cache->handles[handle_idx] |= C_INVALID_MASK;

        // Set free head to point at invalidated handle
        cache->free_head = _make_handle(0, handle_idx);
    }

    --cache->current_used;
}

void* cache_get(struct Cache* cache, int handle)
{
    if(!_check_handle(cache, handle))
    {
        return NULL;
    }

    return _get_item(cache, handle);
}

// ----- CACHE ITERATOR -----

struct CacheIt cache_begin(struct Cache* cache)
{
    struct CacheIt it;
    it.cache = cache;
    it.current_idx = -1;
    it = cache_it_next(it);
    return it;
}

struct CacheIt cache_end(struct Cache* cache)
{
    struct CacheIt it;
    it.cache = cache;
    it.current_idx = cache->max_used;
    return it;
}

struct CacheIt cache_it_next(struct CacheIt it)
{
    ++it.current_idx;

    while(it.current_idx < it.cache->max_used)
    {
        if(_check_valid(it.cache->handles[it.current_idx]))
        {
            return it;
        }

        ++it.current_idx;
    }

    return cache_end(it.cache);
}

bool cache_it_eq(struct CacheIt lhs, struct CacheIt rhs)
{
    return (lhs.current_idx == rhs.current_idx &&
            lhs.cache       == rhs.cache);
}

void* cache_it_get(struct CacheIt it)
{
    return cache_get(it.cache, it.cache->handles[it.current_idx]);
}
