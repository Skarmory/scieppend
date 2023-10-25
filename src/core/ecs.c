#include "scieppend/core/ecs.h"

#include "scieppend/core/array.h"
#include "scieppend/core/array_threadsafe.h"
#include "scieppend/core/cache.h"
#include "scieppend/core/cache_threadsafe.h"
#include "scieppend/core/cache_map.h"
#include "scieppend/core/container_common.h"
#include "scieppend/core/string.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

// TODO: These are just random values for debug purposes
#define ENTITIES_MAX 1024
#define SYSTEMS_MAX 1024
#define COMPONENT_TYPES_MAX 256

#define DEFAULT_SYSTEM_ENTITIES_MAX 32
#define DEFAULT_SYSTEM_REQUIRED_COMPONENTS_MAX 4
#define DEFAULT_ENTITY_COMPONENTS_MAX 8

// Pre-computed hash of "__NullComponentType" string
const int C_NULL_COMPONENT_TYPE = 2025596145;
// Pre-computed hash of "__NullSystemType" string
const int C_NULL_SYSTEM_TYPE = -764810385;
const int C_NULL_ENTITY_HANDLE = 0xffffffff;

// -------------------- STRUCTS --------------------

struct _Entity
{
    struct Array_ThreadSafe components; // Array_ThreadSafe<_ComponentLookup>
};

struct _ComponentLookup
{
    ComponentHandle     id;
    ComponentTypeHandle type_id;
};

struct _System
{
    struct string           name;
    system_update_fn        update_func;
    struct Array_ThreadSafe entities;            // Array_ThreadSafe<EntityHandle>
    struct Array            required_components; // Array<ComponentTypeHandle>
    ObserverHandle          observer;
};

struct _ComponentCache
{
    ComponentTypeHandle     component_type_id;
    struct Cache_ThreadSafe components;        // Cache_ThreadSafe<T>
    struct Cache            component_locks;   // Cache<RWLock>

    struct Event component_added_event;
    struct Event component_removed_event;
};

struct _ComponentCacheNewArgs
{
    int component_type_id;
    int item_size;
    int capacity;
    alloc_fn alloc_func;
    free_fn  free_func;
};

struct _SystemNewArgs
{
    const struct string* name;
    system_update_fn     update_func;
    struct Array*        required_components;
};

static struct _ECS
{
    struct Cache_ThreadSafe entities;         // Cache_ThreadSafe<_Entity>
    struct CacheMap         systems;          // CacheMap<_System>
    struct CacheMap         component_caches; // CacheMap<_ComponentCache>

    struct Event entity_created_event;
    struct Event entity_destroyed_event;
} _ecs;

// -------------------- INTERNAL FUNCS --------------------

static int _compare_entity_handle(const void* lhs, const void* rhs)
{
    EntityHandle _lhs = *(EntityHandle*)lhs;
    EntityHandle _rhs = *(EntityHandle*)rhs;
    return _lhs == _rhs;
}

static int _compare_component_type(const void* lhs, const void* rhs)
{
    ComponentTypeHandle _lhs = *(ComponentTypeHandle*)lhs;
    ComponentTypeHandle _rhs = *(ComponentTypeHandle*)rhs;
    return _lhs == _rhs;
}

static int _compare_component_lookup_by_type(const void* lhs, const void* rhs)
{
    const struct _ComponentLookup* _lhs = lhs;
    return _compare_component_type(&_lhs->type_id, rhs);
}

static void _lock_init(void* lock, [[maybe_unused]] void* _)
{
    rwlock_init((struct RWLock*)lock);
}

static void _lock_uninit(void* lock)
{
    rwlock_uninit((struct RWLock*)lock);
}

static void _component_cache_alloc(void* new_comp_cache, void* new_cache_args)
{
    struct _ComponentCache* _new_comp_cache = new_comp_cache;
    struct _ComponentCacheNewArgs* _new_cache_args = new_cache_args;

    _new_comp_cache->component_type_id = _new_cache_args->component_type_id;

    event_init(&_new_comp_cache->component_added_event);
    event_init(&_new_comp_cache->component_removed_event);

    cache_ts_init(
        &_new_comp_cache->components,
        _new_cache_args->item_size,
        _new_cache_args->capacity,
        _new_cache_args->alloc_func,
        _new_cache_args->free_func
    );

    cache_init(&_new_comp_cache->component_locks, sizeof(struct RWLock), _new_cache_args->capacity, &_lock_init, &_lock_uninit);
}

static void _component_cache_free(void* comp_cache)
{
    struct _ComponentCache* _comp_cache = comp_cache;

    event_uninit(&_comp_cache->component_added_event);
    event_uninit(&_comp_cache->component_removed_event);
    cache_ts_uninit(&_comp_cache->components);
    cache_uninit(&_comp_cache->component_locks);
}

static void _entity_alloc(void* new_entity, [[maybe_unused]] void* new_args)
{
    struct _Entity* entity = new_entity;
    array_ts_init(&entity->components, sizeof(struct _ComponentLookup), DEFAULT_ENTITY_COMPONENTS_MAX, NULL, NULL);
}

static void _entity_free(void* entity)
{
    struct _Entity* _entity = entity;
    array_ts_uninit(&_entity->components);
}

static void* _entity_get_component(struct _Entity* entity, int component_type_id, bool write)
{
    struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);

    if(!comp_cache)
    {
        // TODO: Log debug error
        return NULL;
    }

    struct _ComponentLookup component_lu;
    int found_idx = -1;
    rwlock_read_lock(&entity->components.lock);
    {
        found_idx = array_find(&entity->components.array, &component_type_id, &_compare_component_lookup_by_type);
        if(found_idx != -1)
        {
            component_lu = *(struct _ComponentLookup*)array_get(&entity->components.array, found_idx);
        }
    }
    rwlock_read_unlock(&entity->components.lock);

    if(found_idx == -1)
    {
        return NULL;
    }

    void* component = NULL;
    rwlock_read_lock(&comp_cache->components.lock);
    {
        struct RWLock* lock = cache_get(&comp_cache->component_locks, component_lu.id);

        // The locking here must be unlocked by calling the unget function.
        if(write)
        {
            rwlock_write_lock(lock);
        }
        else
        {
            rwlock_read_lock(lock);
        }

        component = cache_get(&comp_cache->components.cache, component_lu.id);
    }
    rwlock_read_unlock(&comp_cache->components.lock);

    return component;
}

static void _entity_unget_component(EntityHandle entity_handle, const ComponentTypeHandle component_type_handle, bool write)
{
    struct _ComponentCache* component_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_handle);

    rwlock_read_lock(&_ecs.entities.lock);
    {
        struct _Entity* entity = cache_get(&_ecs.entities.cache, entity_handle);

        rwlock_read_lock(&entity->components.lock);
        {
            int idx = array_find(&entity->components.array, &component_type_handle, &_compare_component_lookup_by_type);
            if(idx != -1)
            {
                struct _ComponentLookup* component = array_get(&entity->components.array, idx);

                rwlock_read_lock(&component_cache->components.lock);
                {
                    struct RWLock* component_lock = cache_get(&component_cache->component_locks, component->id);

                    if(write)
                    {
                        rwlock_write_unlock(component_lock);
                    }
                    else
                    {
                        rwlock_read_unlock(component_lock);
                    }
                }
                rwlock_read_unlock(&component_cache->components.lock);
            }
        }
        rwlock_read_unlock(&entity->components.lock);
    }
    rwlock_read_unlock(&_ecs.entities.lock);
}

static bool _system_check_entity_add(struct _System* system, EntityHandle handle)
{
    bool added = true;

    // Check the entity has all the required components for this system
    rwlock_read_lock(&_ecs.entities.lock);
    {
        struct _Entity* entity = cache_get(&_ecs.entities.cache, handle);
        if(entity)
        {
            rwlock_read_lock(&entity->components.lock);
            {
                for(int i = 0; i < array_count(&system->required_components); ++i)
                {
                    ComponentTypeHandle* required_component_type_id = array_get(&system->required_components, i);

                    int found_idx = array_find(&entity->components.array, required_component_type_id, &_compare_component_lookup_by_type);
                    if(found_idx == -1)
                    {
                        added = false;
                        break;
                    }
                }
            }
            rwlock_read_unlock(&entity->components.lock);
        }
    }
    rwlock_read_unlock(&_ecs.entities.lock);

    if(added)
    {
        array_ts_add(&system->entities, &handle);
    }

    return added;
}

static void _system_event_callback([[maybe_unused]] struct Event* sender, void* observer_data, void* event_args)
{
    struct _System* system = observer_data;
    enum ECSEventType event_type = ((struct EntityEventArgs*)event_args)->event_type;

    switch(event_type)
    {
        case EVENT_COMPONENT_ADDED:
            {
                struct ComponentEventArgs* args = event_args;
                _system_check_entity_add(system, args->entity_handle);
            }
            break;
        case EVENT_COMPONENT_REMOVED:
        case EVENT_ENTITY_DESTROYED:
            {
                struct EntityEventArgs* args = event_args;
                array_ts_find_and_remove(&system->entities, &args->entity_handle, &_compare_entity_handle);
            }
            break;
        case EVENT_ENTITY_CREATED:
            // TODO handle
            break;
    }
}

static void _system_alloc(void* new_system, void* new_system_args)
{
    struct _System* _new_system  = new_system;
    struct _SystemNewArgs* _args = new_system_args;

    string_init(&_new_system->name, _args->name->buffer);
    _new_system->update_func = _args->update_func;
    array_ts_init(&_new_system->entities, sizeof(EntityHandle), DEFAULT_SYSTEM_ENTITIES_MAX, NULL, NULL);
    array_copy(&_new_system->required_components, _args->required_components);
    _new_system->observer = observer_create(_new_system, &_system_event_callback);

    // Go through required components and set up event callbacks
    for(int i = 0; i < array_count(&_new_system->required_components); ++i)
    {
        int* component_type_id = array_get(&_new_system->required_components, i);
        struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, *component_type_id);
        event_register_observer(&comp_cache->component_added_event, _new_system->observer);
        event_register_observer(&comp_cache->component_removed_event, _new_system->observer);
    }
}

static void _system_free(void* system)
{
    struct _System* _system = system;
    string_uninit(&_system->name);
    array_ts_uninit(&_system->entities);

    for(int i = 0; i < array_count(&_system->required_components); ++i)
    {
        ComponentTypeHandle* component_type_id = array_get(&_system->required_components, i);
        struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, *component_type_id);
        event_deregister_observer(&comp_cache->component_added_event, _system->observer);
        event_deregister_observer(&comp_cache->component_removed_event, _system->observer);
    }

    observer_destroy(_system->observer);
    array_uninit(&_system->required_components);
}

// -------------------- EXTERNAL FUNCS --------------------

void ecs_init(void)
{
    cache_ts_init(&_ecs.entities, sizeof(struct _Entity), ENTITIES_MAX, &_entity_alloc, &_entity_free);
    cache_map_init(&_ecs.systems, sizeof(struct _System), SYSTEMS_MAX, &_system_alloc, &_system_free);
    cache_map_init(&_ecs.component_caches, sizeof(struct _ComponentCache), COMPONENT_TYPES_MAX, &_component_cache_alloc, &_component_cache_free);

    event_init(&_ecs.entity_created_event);
    event_init(&_ecs.entity_destroyed_event);
}

void ecs_uninit(void)
{
    cache_ts_uninit(&_ecs.entities);
    cache_map_uninit(&_ecs.systems);
    cache_map_uninit(&_ecs.component_caches);

    event_uninit(&_ecs.entity_created_event);
    event_uninit(&_ecs.entity_destroyed_event);
}

int ecs_entities_count(void)
{
    return cache_ts_count(&_ecs.entities);
}

int ecs_component_types_count(void)
{
    return cache_map_count(&_ecs.component_caches);
}

int ecs_components_count(const struct string* name)
{
    int component_type_id = string_hash(name);
    struct _ComponentCache* comp_cache =  cache_map_get_hashed(&_ecs.component_caches, component_type_id);
    return cache_ts_count(&comp_cache->components);
}

int ecs_systems_count(void)
{
    return cache_map_count(&_ecs.systems);
}

EntityHandle entity_create(void)
{
    EntityHandle handle = cache_ts_emplace(&_ecs.entities, NULL);

    if(handle == C_NULL_ENTITY_HANDLE)
    {
        // TODO: Log error
    }

    return handle;
}

void entity_destroy(EntityHandle id)
{
    // Lock entities to keep our Entity pointer alive
    rwlock_read_lock(&_ecs.entities.lock);
    {
        // Need to lock the entity components so we can remove them all.
        struct _Entity* entity = cache_get(&_ecs.entities.cache, id);
        if(entity)
        {
            rwlock_write_lock(&entity->components.lock);
            {
                // Loop over components and remove them
                for(int i = array_count(&entity->components.array) - 1; i != -1; --i)
                {
                    struct _ComponentLookup* component = array_get(&entity->components.array, i);
                    struct _ComponentCache* component_cache = cache_map_get_hashed(&_ecs.component_caches, component->type_id);

                    struct ComponentEventArgs args;
                    args.event_type = EVENT_COMPONENT_REMOVED;
                    args.entity_handle = id;
                    args.component_type = component->type_id;

                    rwlock_write_lock(&component_cache->components.lock);
                    {
                        struct RWLock* component_lock = cache_get(&component_cache->component_locks, component->id);
                        rwlock_write_lock(component_lock); 
                        {
                            // Lock the component to stop access while being destroyed.
                            cache_remove(&component_cache->components.cache, component->id);
                        }
                        rwlock_write_unlock(component_lock);

                        cache_remove(&component_cache->component_locks, component->id);
                        array_remove_at(&entity->components.array, i);
                    }
                    rwlock_write_unlock(&component_cache->components.lock);

                    event_send(&component_cache->component_removed_event, &args);
                }
            }
            rwlock_write_unlock(&entity->components.lock);
        }
    }
    rwlock_read_unlock(&_ecs.entities.lock);

    cache_ts_remove(&_ecs.entities, id);

    struct EntityEventArgs args;
    args.event_type = EVENT_ENTITY_DESTROYED;
    args.entity_handle = id;
    event_send(&_ecs.entity_destroyed_event, &args);
}

void entity_add_component(EntityHandle entity_handle, const int component_type_id)
{
    struct _ComponentLookup new_component;
    struct _ComponentCache* component_cache = NULL;
    bool added = false;

    rwlock_read_lock(&_ecs.entities.lock); // Lock entity cache
    {
        struct _Entity* entity = cache_get(&_ecs.entities.cache, entity_handle);
        component_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);

        if(entity && component_cache)
        {
            void* found = array_ts_find_and_get(&entity->components, &component_type_id, &_compare_component_lookup_by_type);
            if(!found)
            {
                // Cannot add a duplicate component type, must be unique
                rwlock_write_lock(&component_cache->components.lock);
                {
                    struct RWLock* component_lock = cache_get(&component_cache->component_locks, cache_emplace(&component_cache->component_locks, NULL));
                    rwlock_write_lock(component_lock);
                    {
                        new_component.type_id = component_type_id;
                        new_component.id = cache_emplace(&component_cache->components.cache, NULL);
                        array_ts_add(&entity->components, &new_component);
                        added = true;
                    }
                    rwlock_write_unlock(component_lock);
                }
                rwlock_write_unlock(&component_cache->components.lock);
            }
        }
    }
    rwlock_read_unlock(&_ecs.entities.lock);

    if(added)
    {
        struct ComponentEventArgs args;
        args.event_type = EVENT_COMPONENT_ADDED;
        args.entity_handle = entity_handle;
        args.component_type = new_component.type_id;
        event_send(&component_cache->component_added_event, &args);
    }
}

void entity_add_component_by_name(EntityHandle entity_handle, const struct string* component_name)
{
    int component_type_id = string_hash(component_name);
    entity_add_component(entity_handle, component_type_id);
}

void entity_remove_component(EntityHandle entity_handle, const int component_type_id)
{
    rwlock_read_lock(&_ecs.entities.lock);
    {
        struct _Entity* entity = cache_ts_get(&_ecs.entities, entity_handle);
        struct _ComponentCache* component_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);

        if(entity && component_cache)
        {
            rwlock_write_lock(&entity->components.lock);
            {
                for(int i = 0; i < array_count(&entity->components.array); ++i)
                {
                    struct _ComponentLookup* entity_component = array_get(&entity->components.array, i);
                    if(entity_component->type_id == component_type_id)
                    {
                        rwlock_write_lock(&component_cache->components.lock);
                        {
                            struct RWLock* component_lock = cache_get(&component_cache->component_locks, entity_component->id);
                            rwlock_write_lock(component_lock);
                            {
                                cache_remove(&component_cache->components.cache, entity_component->id);
                            }
                            rwlock_write_unlock(component_lock);

                            cache_remove(&component_cache->component_locks, entity_component->id);
                            array_remove_at(&entity->components.array, i);
                        }
                        rwlock_write_unlock(&component_cache->components.lock);
                        break;
                    }
                }
            }
            rwlock_write_unlock(&entity->components.lock);
        }
    }
    rwlock_read_unlock(&_ecs.entities.lock);
}

void entity_remove_component_by_name(EntityHandle entity_handle, const struct string* component_name)
{
    int component_type_id = string_hash(component_name);
    entity_remove_component(entity_handle, component_type_id);
}

void* entity_get_component(EntityHandle entity_id, int component_type_id)
{
    void* component = NULL;

    rwlock_read_lock(&_ecs.entities.lock);
    {
        struct _Entity* entity = cache_get(&_ecs.entities.cache, entity_id);
        if(entity)
        {
            component = _entity_get_component(entity, component_type_id, true);
        }
    }
    rwlock_read_unlock(&_ecs.entities.lock);

    return component;
}

const void* entity_get_readonly_component(EntityHandle entity_id, const int component_type_id)
{
    void* component = NULL;

    rwlock_read_lock(&_ecs.entities.lock);
    {
        struct _Entity* entity = cache_get(&_ecs.entities.cache, entity_id);
        if(entity)
        {
            component = _entity_get_component(entity, component_type_id, false);
        }
    }
    rwlock_read_unlock(&_ecs.entities.lock);

    return component;
}

void* entity_get_component_by_name(EntityHandle entity_id, const struct string* component_name)
{
    int component_type_id = string_hash(component_name);
    return entity_get_component(entity_id, component_type_id);
}

void entity_unget_component(EntityHandle entity_handle, const ComponentTypeHandle component_type_handle)
{
    _entity_unget_component(entity_handle, component_type_handle, true);
}

void entity_unget_readonly_component(EntityHandle entity_handle, const ComponentTypeHandle component_type_handle)
{
    _entity_unget_component(entity_handle, component_type_handle, false);
}

int entity_component_count(EntityHandle id)
{
    int count = -1;

    rwlock_read_lock(&_ecs.entities.lock);
    {
        struct _Entity* entity = cache_get(&_ecs.entities.cache, id);
        if(entity)
        {
            count = array_ts_count(&entity->components);
        }
    }
    rwlock_read_unlock(&_ecs.entities.lock);

    return count;
}

int component_type_register(const struct string* name, int component_type_size_bytes)
{
    int component_type_id = string_hash(name);
    if(cache_map_get_hashed(&_ecs.component_caches, component_type_id))
    {
        // TODO: Log warning
        return C_NULL_COMPONENT_TYPE;
    }

    struct _ComponentCacheNewArgs args;
    args.component_type_id = component_type_id;
    args.item_size = component_type_size_bytes;
    args.capacity = 1024;
    args.alloc_func = NULL;
    args.free_func = NULL;

    cache_map_emplace(&_ecs.component_caches, name->buffer, name->size, &args);

    return component_type_id;
}

void component_type_added_register_observer(const int component_type_id, ObserverHandle observer)
{
    struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);
    if(comp_cache == NULL)
    {
        // TODO log error
        return;
    }

    event_register_observer(&comp_cache->component_added_event, observer);
}

void component_type_added_deregister_observer(const int component_type_id, ObserverHandle observer)
{
    struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);
    if(comp_cache == NULL)
    {
        // TODO log error
        return;
    }

    event_deregister_observer(&comp_cache->component_added_event, observer);
}

void component_type_removed_register_observer(const int component_type_id, ObserverHandle observer)
{
    struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);
    if(comp_cache == NULL)
    {
        // TODO log error
        return;
    }

    event_register_observer(&comp_cache->component_removed_event, observer);
}

void component_type_removed_deregister_observer(const int component_type_id, ObserverHandle observer)
{
    struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);
    if(comp_cache == NULL)
    {
        // TODO log error
        return;
    }

    event_deregister_observer(&comp_cache->component_removed_event, observer);
}

int system_register(const struct string* name, system_update_fn update_func, struct Array* required_components)
{
    int system_type_id = string_hash(name);
    if(cache_map_get_hashed(&_ecs.systems, system_type_id) != NULL)
    {
        // TODO: Log error
        return C_NULL_SYSTEM_TYPE;
    }

    struct _SystemNewArgs args;
    args.name = name;
    args.update_func = update_func;
    args.required_components = required_components;

    cache_map_emplace(&_ecs.systems, name->buffer, name->size, &args);

    return system_type_id;
}

void systems_update(void)
{
    struct It it = cache_map_begin(&_ecs.systems);
    struct It eit = cache_map_end(&_ecs.systems);
    for(; !it_eq(&it, &eit); cache_map_it_next(&it))
    {
        struct _System* sys = cache_map_it_get(&it);

        rwlock_read_lock(&sys->entities.lock);
        {
            for(int i = 0; i < array_count(&sys->entities.array); ++i)
            {
                sys->update_func(*(EntityHandle*)array_get(&sys->entities.array, i));
            }
        }
        rwlock_read_unlock(&sys->entities.lock);
    }
}

int system_entity_count(const struct string* system_name)
{
    int system_type_id = string_hash(system_name);

    struct _System* system = cache_map_get_hashed(&_ecs.systems, system_type_id);
    if(system == NULL)
    {
        // TODO log error
        return -1;
    }

    return array_ts_count(&system->entities);
}
