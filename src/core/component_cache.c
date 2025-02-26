#include "scieppend/core/component_cache.h"

#include "scieppend/core/ecs_events.h"

#include <assert.h>
#include <stdlib.h>

// ---------- INTERNAL FUNCS ----------

// ---------- EXTERNAL FUNCS ----------

struct ComponentCache* component_cache_new(ComponentTypeHandle type_handle, int bytes, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    struct ComponentCache* component_cache = malloc(sizeof(struct ComponentCache));
    component_cache_init(component_cache, type_handle, bytes, capacity, alloc_func, free_func);
    return component_cache;
}

void component_cache_init(struct ComponentCache* component_cache, ComponentTypeHandle type_handle, int bytes, int capacity, alloc_fn alloc_func, free_fn free_func)
{
    component_cache->component_type_handle = type_handle;
    cache_ts_init(&component_cache->components, bytes, capacity, alloc_func, free_func);
    cache_init(&component_cache->component_locks, sizeof(struct RWLock), capacity, rwlock_init_wrapper, rwlock_uninit_wrapper);
    event_init(&component_cache->component_added_event);
    event_init(&component_cache->component_removed_event);
}

void component_cache_init_wrapper(void* component_cache, const void* args)
{
    struct ComponentCache* cache = component_cache;
    const struct ComponentCacheArgs* _args = args;
    component_cache_init(cache, _args->type_handle, _args->bytes, _args->capacity, _args->alloc_func, _args->free_func);
}

void component_cache_free(struct ComponentCache* component_cache)
{
    component_cache_uninit(component_cache);
    free(component_cache);
}

void component_cache_uninit(struct ComponentCache* component_cache)
{
    event_uninit(&component_cache->component_removed_event);
    event_uninit(&component_cache->component_added_event);
    cache_uninit(&component_cache->component_locks);
    cache_ts_uninit(&component_cache->components);
}

void component_cache_uninit_wrapper(void* component_cache)
{
    struct ComponentCache* cache = component_cache;
    component_cache_uninit(cache);
}

ComponentHandle component_cache_emplace_component(struct ComponentCache* component_cache, const void* args)
{
    cache_ts_lock(&component_cache->components, WRITE);

    ComponentHandle component_handle = cache_emplace(&component_cache->components.cache, args);
    int lock_handle = cache_emplace(&component_cache->component_locks, NULL);

    assert(component_handle == lock_handle && "component_cache_emplace_component: component handle and lock handle do not match.");

    cache_ts_unlock(&component_cache->components, WRITE);

    return component_handle;
}

void component_cache_remove_component(struct ComponentCache* component_cache, ComponentHandle handle)
{
    cache_ts_lock(&component_cache->components, WRITE);

    // Lock the component for write, so we can remove it safely
    struct RWLock* lock = cache_get(&component_cache->component_locks, handle);
    rwlock_lock(lock, WRITE);
    rwlock_set_kill(lock);
    cache_remove(&component_cache->components.cache, handle);
    rwlock_unlock(lock, WRITE);

    cache_remove(&component_cache->component_locks, handle);
    cache_ts_unlock(&component_cache->components, WRITE);
}

void component_cache_lock(const struct ComponentCache* component_cache, bool write)
{
    cache_ts_lock(&component_cache->components, write);
}

void component_cache_unlock(const struct ComponentCache* component_cache, bool write)
{
    cache_ts_unlock(&component_cache->components, write);
}

void component_cache_lock_component(const struct ComponentCache* component_cache, const ComponentHandle handle, bool write)
{
    cache_ts_lock(&component_cache->components, READ);
    struct RWLock* component_lock = cache_get(&component_cache->component_locks, handle);
    rwlock_lock(component_lock, write);
    cache_ts_unlock(&component_cache->components, READ);
}

void component_cache_unlock_component(const struct ComponentCache* component_cache, const ComponentHandle handle, bool write)
{
    cache_ts_lock(&component_cache->components, READ);
    struct RWLock* component_lock = cache_get(&component_cache->component_locks, handle);
    rwlock_unlock(component_lock, write);
    cache_ts_unlock(&component_cache->components, READ);
}

void component_cache_register_observer(struct ComponentCache* component_cache, const ObserverHandle observer_handle)
{
    event_register_observer(&component_cache->component_added_event, observer_handle);
    event_register_observer(&component_cache->component_removed_event, observer_handle);
}

void component_cache_deregister_observer(struct ComponentCache* component_cache, const ObserverHandle observer_handle)
{
    event_deregister_observer(&component_cache->component_added_event, observer_handle);
    event_deregister_observer(&component_cache->component_removed_event, observer_handle);
}

void component_cache_send(const struct ComponentCache* component_cache, enum ECSEventType event_type, const EntityHandle entity_handle, const ComponentHandle component_handle)
{
    switch(event_type)
    {
        case EVENT_COMPONENT_REMOVED:
            ecs_event_send_component_event(&component_cache->component_removed_event, EVENT_COMPONENT_REMOVED, entity_handle, component_cache->component_type_handle, component_handle);
            break;
        case EVENT_COMPONENT_ADDED:
            ecs_event_send_component_event(&component_cache->component_added_event, EVENT_COMPONENT_ADDED, entity_handle, component_cache->component_type_handle, component_handle);
            break;
        default:
            break;
    }
}

void* component_cache_get_component(const struct ComponentCache* component_cache, const ComponentHandle handle, bool write)
{
    cache_ts_lock(&component_cache->components, READ);

    void* item = cache_get(&component_cache->components.cache, handle);

    if(item != NULL)
    {
        struct RWLock* component_lock = cache_get(&component_cache->component_locks, handle);
        rwlock_lock(component_lock, write);
    }

    cache_ts_unlock(&component_cache->components, READ);

    return item;
}

void component_cache_unget_component(const struct ComponentCache* component_cache, const ComponentHandle handle, bool write)
{
    cache_ts_lock(&component_cache->components, READ);

    void* item = cache_get(&component_cache->components.cache, handle);

    if(item != NULL)
    {
        struct RWLock* component_lock = cache_get(&component_cache->component_locks, handle);
        rwlock_unlock(component_lock, write);
    }

    cache_ts_unlock(&component_cache->components, READ);
}

int component_cache_count(const struct ComponentCache* component_cache)
{
    return cache_size(&component_cache->components.cache);
}
