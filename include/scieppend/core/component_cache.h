#ifndef SCIEPPEND_CORE_COMPONENT_CACHE_H
#define SCIEPPEND_CORE_COMPONENT_CACHE_H

#include "scieppend/core/cache.h"
#include "scieppend/core/cache_threadsafe.h"
#include "scieppend/core/container_common.h"
#include "scieppend/core/ecs_defs.h"
#include "scieppend/core/event.h"

struct Cache_ThreadSafe;

struct ComponentCache
{
    ComponentTypeHandle     component_type_handle;
    struct Cache_ThreadSafe components;
    struct Cache            component_locks;
    struct Event            component_added_event;
    struct Event            component_removed_event;
};

struct ComponentCacheArgs
{
    ComponentTypeHandle type_handle;
    int                 bytes;
    int                 capacity;
    alloc_fn            alloc_func;
    free_fn             free_func;
};

struct ComponentCache* component_cache_new(ComponentTypeHandle type_handle, int bytes, int capacity, alloc_fn alloc_func, free_fn free_func);
void component_cache_init(struct ComponentCache* component_cache, ComponentTypeHandle type_handle, int bytes, int capacity, alloc_fn alloc_func, free_fn free_func);
void component_cache_init_wrapper(void* component_cache, const void* args);
void component_cache_free(struct ComponentCache* component_cache);
void component_cache_uninit(struct ComponentCache* component_cache);
void component_cache_uninit_wrapper(void* component_cache);

// Mutators
ComponentHandle component_cache_emplace_component(struct ComponentCache* component_cache, const void* args);
void component_cache_remove_component(struct ComponentCache* component_cache, ComponentHandle handle);

void component_cache_lock(const struct ComponentCache* component_cache, bool write);
void component_cache_unlock(const struct ComponentCache* component_cache, bool write);

void component_cache_register_observer(struct ComponentCache* component_cache, const ObserverHandle observer_handle);
void component_cache_deregister_observer(struct ComponentCache* component_cache, const ObserverHandle observer_handle);
void component_cache_send(const struct ComponentCache* component_cache, enum ECSEventType event_type, const EntityHandle entity_handle, const ComponentHandle component_handle);

// Accessors
void* component_cache_get_component(const struct ComponentCache* component_cache, const ComponentHandle handle, bool write);
void component_cache_unget_component(const struct ComponentCache* component_cache, const ComponentHandle handle, bool write);
int component_cache_count(const struct ComponentCache* component_cache);

#endif
