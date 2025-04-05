#include "scieppend/core/ecs_world.h"

#include "scieppend/core/array.h"
#include "scieppend/core/cache_map.h"
#include "scieppend/core/cache_threadsafe.h"
#include "scieppend/core/component.h"
#include "scieppend/core/component_cache.h"
#include "scieppend/core/ecs_events.h"
#include "scieppend/core/entity.h"
#include "scieppend/core/string.h"
#include "scieppend/core/system.h"

#include <assert.h>
#include <stdlib.h>

// Pre-computed hash of "__NullComponentType" string
const int C_NULL_COMPONENT_TYPE = NULL_COMPONENT_TYPE_PREHASH_MACRO;
const int C_NULL_COMPONENT_HANDLE = 0xffffffff;

struct ECSWorld
{
    struct Cache_ThreadSafe entities;         // Cache_ThreadSafe<_Entity>
    struct CacheMap         systems;          // CacheMap<System>
    struct CacheMap         component_caches; // CacheMap<ComponentCache>

    struct Event entity_created_event;
    struct Event entity_destroyed_event;
};

// ---------- EXTERNAL FUNCTIONS ----------

struct ECSWorld* ecs_world_new(void)
{
    struct ECSWorld* new_ecs_world = malloc(sizeof(struct ECSWorld));
    cache_map_init(&new_ecs_world->component_caches, sizeof(struct ComponentCache), 32, &component_cache_init_wrapper, &component_cache_uninit_wrapper);
    cache_map_init(&new_ecs_world->systems, sizeof(struct System), 32, &system_init_wrapper, &system_uninit_wrapper);
    cache_ts_init(&new_ecs_world->entities, sizeof(struct Entity), 1024, &entity_init_wrapper, &entity_uninit_wrapper);

    event_init(&new_ecs_world->entity_created_event);
    event_init(&new_ecs_world->entity_destroyed_event);

    return new_ecs_world;
}

void ecs_world_free(struct ECSWorld* world)
{
    event_uninit(&world->entity_destroyed_event);
    event_uninit(&world->entity_created_event);

    cache_ts_uninit(&world->entities);
    cache_map_uninit(&world->systems);
    cache_map_uninit(&world->component_caches);

    free(world);
}

int ecs_world_entities_count(const struct ECSWorld* world)
{
    return cache_ts_count(&world->entities);
}

int ecs_world_systems_count(const struct ECSWorld* world)
{
    return cache_map_count(&world->systems);
}

int ecs_world_component_types_count(const struct ECSWorld* world)
{
    return cache_map_count(&world->component_caches);
}

int ecs_world_components_count(const struct ECSWorld* world, ComponentTypeHandle component_type_handle)
{
    struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
    return component_cache_count(component_cache);
}

EntityHandle ecs_world_create_entity(struct ECSWorld* world)
{
    return cache_ts_emplace(&world->entities, world);
}

void ecs_world_destroy_entity(struct ECSWorld* world, EntityHandle entity_handle)
{
    cache_ts_lock(&world->entities, WRITE);

    struct Entity* entity = cache_get(&world->entities.cache, entity_handle);
    if(entity)
    {
        entity_lock(entity, WRITE);
        rwlock_set_kill(&entity->components.lock);

        const struct Array* components = entity_get_components(entity);
        for(int i = 0; i < array_count(components); ++i)
        {
            struct ComponentLookup* lookup = array_get(components, i);
            struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, lookup->component_type_handle);
            component_cache_remove_component(component_cache, lookup->component_handle);
        }

        entity_unlock(entity, WRITE);
        cache_remove(&world->entities.cache, entity_handle);

        ecs_event_send_entity_event(&world->entity_destroyed_event, EVENT_ENTITY_DESTROYED, entity_handle);
    }

    cache_ts_unlock(&world->entities, WRITE);
}

void ecs_world_entity_add_component(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle)
{
    struct ComponentCache* component_cache = NULL;
    ComponentHandle component_handle = C_NULL_COMPONENT_HANDLE;

    cache_ts_lock(&world->entities, READ);

    struct Entity* entity = cache_get(&world->entities.cache, entity_handle);
    if(entity)
    {
        if (!entity_has_component(entity, component_type_handle))
        {
            component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
            component_handle = component_cache_emplace_component(component_cache, NULL);
            entity_add_component(entity, component_handle, component_type_handle);
        }
    }

    if(component_handle != C_NULL_COMPONENT_HANDLE)
    {
        component_cache_send(component_cache, EVENT_COMPONENT_ADDED, entity_handle, component_handle);
    }

    cache_ts_unlock(&world->entities, READ);
}

void ecs_world_entity_remove_component(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle)
{
    struct ComponentCache* component_cache = NULL;
    ComponentHandle component_handle = C_NULL_COMPONENT_HANDLE;

    cache_ts_lock(&world->entities, READ);

    struct Entity* entity = cache_get(&world->entities.cache, entity_handle);
    if(entity)
    {
        component_handle = entity_get_component(entity, component_type_handle);
        if(component_handle != C_NULL_COMPONENT_HANDLE)
        {
            entity_remove_component(entity, component_type_handle);
            component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
            component_cache_remove_component(component_cache, component_handle);
        }
    }

    cache_ts_unlock(&world->entities, READ);

    if(component_handle != C_NULL_COMPONENT_HANDLE)
    {
        component_cache_send(component_cache, EVENT_COMPONENT_REMOVED, entity_handle, component_handle);
    }
}

int ecs_world_entity_components_count(struct ECSWorld* world, EntityHandle entity_handle)
{
    int count = 0;

    cache_ts_lock(&world->entities, READ);

    struct Entity* entity = cache_get(&world->entities.cache, entity_handle);
    if(entity)
    {
        count = entity_components_count(entity);
    }

    cache_ts_unlock(&world->entities, READ);

    return count;
}

ComponentHandle ecs_world_entity_get_component_handle(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle)
{
    ComponentHandle component_handle = C_NULL_COMPONENT_HANDLE;

    cache_ts_lock(&world->entities, READ);

    struct Entity* entity = cache_get(&world->entities.cache, entity_handle);
    if(entity)
    {
        component_handle = entity_get_component(entity, component_type_handle);
    }

    cache_ts_unlock(&world->entities, READ);

    return component_handle;
}

bool ecs_world_entity_has_component(struct ECSWorld* world, EntityHandle entity_handle, const ComponentTypeHandle component_type_handle)
{
    bool has = false;

    cache_ts_lock(&world->entities, READ);

    struct Entity* entity = cache_get(&world->entities.cache, entity_handle);
    if(entity)
    {
        has = entity_has_component(entity, component_type_handle);
    }

    cache_ts_unlock(&world->entities, READ);

    return has;
}

bool ecs_world_entity_has_components(struct ECSWorld* world, EntityHandle entity_handle, const struct Array* component_type_handles)
{
    bool has = false;

    cache_ts_lock(&world->entities, READ);

    struct Entity* entity = cache_get(&world->entities.cache, entity_handle);
    if(entity)
    {
        has = entity_has_components(entity, component_type_handles);
    }

    cache_ts_unlock(&world->entities, READ);

    return has;
}

void* ecs_world_entity_get_component(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle, bool write)
{
    struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
    if (!component_cache)
    {
        return NULL;
    }

    void* component = NULL;
    cache_ts_lock(&world->entities, READ);

    const struct Entity* entity = cache_get(&world->entities.cache, entity_handle);
    if(entity && entity_lock(entity, READ))
    {
        ComponentHandle component_handle = entity_get_component(entity, component_type_handle);
        if (component_handle != C_NULL_COMPONENT_HANDLE)
        {
            component = component_cache_get_component(component_cache, component_handle, write);
        }

        entity_unlock(entity, READ);
    }

    cache_ts_unlock(&world->entities, READ);

    return component;
}

void ecs_world_entity_unget_component(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle, bool write)
{
    struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
    if (!component_cache)
    {
        return;
    }

    cache_ts_lock(&world->entities, READ);
    {
        struct Entity* entity = cache_get(&world->entities.cache, entity_handle);
        if (entity != NULL && entity_lock(entity, READ))
        {
            ComponentHandle component_handle = entity_get_component(entity, component_type_handle);
            if (component_handle != C_NULL_COMPONENT_HANDLE)
            {
                component_cache_unget_component(component_cache, component_handle, write);
            }

            entity_unlock(entity, READ);
        }
    }
    cache_ts_unlock(&world->entities, READ);
}

// Component functions

void* ecs_world_get_component(struct ECSWorld* world, const ComponentHandle component_handle, const ComponentTypeHandle component_type_handle, bool write)
{
    void* component = NULL;

    if(component_handle != C_NULL_COMPONENT_HANDLE)
    {
        struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
        if (component_cache != NULL)
        {
            component = component_cache_get_component(component_cache, component_handle, write);
        }
    }

    return component;
}

void ecs_world_unget_component(struct ECSWorld* world, const ComponentHandle component_handle, const ComponentTypeHandle component_type_handle, bool write)
{
    if(component_handle != C_NULL_COMPONENT_HANDLE)
    {
        struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
        if(component_cache != NULL)
        {
            component_cache_unget_component(component_cache, component_handle, write);
        }
    }
}

// Component type functions

void ecs_world_component_type_register(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, int bytes)
{
    if(cache_map_get_hashed(&world->component_caches, component_type_handle))
    {
        // TODO: Log warning
        return;
    }

    struct ComponentCacheArgs args;
    args.type_handle = component_type_handle;
    args.bytes = bytes;
    args.capacity = 1024;
    args.alloc_func = NULL;
    args.free_func = NULL;

    cache_map_emplace_hashed(&world->component_caches, component_type_handle, &args);
}

void ecs_world_component_type_lock(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, bool write)
{
    struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
    component_cache_lock(component_cache, write);
}

void ecs_world_component_type_unlock(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, bool write)
{
    struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
    component_cache_unlock(component_cache, write);
}

void ecs_world_component_type_register_observer(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, const ObserverHandle observer)
{
    struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
    component_cache_register_observer(component_cache, observer);
}

void ecs_world_component_type_deregister_observer(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, const ObserverHandle observer)
{
    struct ComponentCache* component_cache = cache_map_get_hashed(&world->component_caches, component_type_handle);
    component_cache_deregister_observer(component_cache, observer);
}

bool ecs_world_component_type_is_registered(struct ECSWorld* world, const ComponentTypeHandle component_type_handle)
{
    return cache_map_get_hashed(&world->component_caches, component_type_handle) != NULL;
}

void ecs_world_system_register(struct ECSWorld* world, const struct string* system_name, const struct Array* required_components, SystemUpdateFn update_func)
{
    if(cache_map_get(&world->systems, system_name->buffer, system_name->size))
    {
        return;
    }

    for(int i = 0; i < array_count(required_components); ++i)
    {
        ComponentTypeHandle ct_h = *(ComponentHandle*)array_get(required_components, i);
        assert(ecs_world_component_type_is_registered(world, ct_h)  && "World does not have component type registered");
    }

    struct SystemInitArgs args;
    args.world = world;
    args.name = system_name;
    args.required_components = required_components;
    args.update_func = update_func;

    struct System* system = cache_map_emplace(&world->systems, system_name->buffer, system_name->size, &args);

    event_register_observer(&world->entity_created_event, system->observer_handle);
    event_register_observer(&world->entity_destroyed_event, system->observer_handle);
}

struct System* ecs_world_get_system(const struct ECSWorld* world, const struct string* system_name)
{
    return cache_map_get(&world->systems, system_name->buffer, system_name->size);
}

void ecs_world_update_systems(const struct ECSWorld* world)
{
    struct It it = cache_map_begin(&world->systems);
    struct It end = cache_map_end(&world->systems);

    for(; !it_eq(&it, &end); cache_map_it_next(&it))
    {
        struct System* system = cache_map_it_get(&it);
        system_process_ecs_commands(system);
    }

    it = cache_map_begin(&world->systems);
    for(; !it_eq(&it, &end); cache_map_it_next(&it))
    {
        struct System* system = cache_map_it_get(&it);
        system_update(system);
    }

    it = cache_map_begin(&world->systems);
    for(; !it_eq(&it, &end); cache_map_it_next(&it))
    {
        struct System* system = cache_map_it_get(&it);
        system_process_ecs_commands(system);
    }
}

int ecs_world_system_entities_count(const struct ECSWorld* world, const struct string* system_name)
{
    const struct System* system = cache_map_get(&world->systems, system_name->buffer,  system_name->size);

    return system_entities_count(system);
}
