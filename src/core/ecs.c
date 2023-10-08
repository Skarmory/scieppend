#include "scieppend/core/ecs.h"

#include "scieppend/core/cache.h"
#include "scieppend/core/cache_map.h"
#include "scieppend/core/container_common.h"
#include "scieppend/core/event.h"
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

// STRUCTS

struct _Entity
{
    struct Array components;
};

struct _Component
{
    ComponentHandle     id;
    ComponentTypeHandle type_id;
};

struct _ComponentCache
{
    ComponentTypeHandle component_type_id;
    struct Event component_added_event;
    struct Event component_removed_event;
    struct Cache components;
};

struct _System
{
    struct string    name;
    system_update_fn update_func;
    struct Array     entities;
    struct Array     required_components;
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
    const struct Array*  required_components;
};

static struct ECS
{
    struct Cache    entities;
    struct CacheMap systems;
    struct CacheMap component_caches;

    struct Event entity_created_event;
    struct Event entity_destroyed_event;
} _ecs;

// INTERNAL FUNCS

static bool _compare_entity_handle(const void* lhs, const void* rhs)
{
    EntityHandle _lhs = *(EntityHandle*)lhs;
    EntityHandle _rhs = *(EntityHandle*)rhs;
    return _lhs == _rhs;
}

static void _component_cache_alloc(void* new_comp_cache, void* new_cache_args)
{
    struct _ComponentCache* _new_comp_cache = new_comp_cache;
    struct _ComponentCacheNewArgs* _new_cache_args = new_cache_args;

    _new_comp_cache->component_type_id = _new_cache_args->component_type_id;

    event_init(&_new_comp_cache->component_added_event);
    event_init(&_new_comp_cache->component_removed_event);

    cache_init(
        &_new_comp_cache->components,
        _new_cache_args->item_size,
        _new_cache_args->capacity,
        _new_cache_args->alloc_func,
        _new_cache_args->free_func
    );
}

static void _component_cache_free(void* comp_cache)
{
    struct _ComponentCache* _comp_cache = comp_cache;
    event_uninit(&_comp_cache->component_added_event);
    event_uninit(&_comp_cache->component_removed_event);
    cache_uninit(&_comp_cache->components);
}

static void _entity_alloc(void* new_entity, [[maybe_unused]] void* new_args)
{
    struct _Entity* entity = new_entity;
    array_init(&entity->components, sizeof(struct _Component), DEFAULT_ENTITY_COMPONENTS_MAX, NULL, NULL);
}

static void _entity_free(void* entity)
{
    struct _Entity* _entity = entity;
    array_uninit(&_entity->components);
}

static void _entity_remove_component(EntityHandle entity_handle, struct _Entity* entity, struct _ComponentCache* comp_cache, struct _Component* component, int component_idx)
{
    cache_remove(&comp_cache->components, component->id);
    array_remove_at(&entity->components, component_idx);
    event_send(&comp_cache->component_removed_event, &entity_handle);
}

static void* _entity_get_component(const struct _Entity* entity, int component_type_id)
{
    struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);

    if(comp_cache == NULL)
    {
        // TODO: Log debug error
        return NULL;
    }

    for(int i = 0; i < array_count(&entity->components); ++i)
    {
        struct _Component* component = array_get(&entity->components, i);

        if(component->type_id == component_type_id)
        {
            return cache_get(&comp_cache->components, component->id);
        }
    }

    return NULL;
}

static bool _system_check_entity_add(struct _System* system, EntityHandle handle)
{
    // Check the entity has all the required components for this system
    struct _Entity* entity = cache_get(&_ecs.entities, handle);
    for(int i = 0; i < array_count(&system->required_components); ++i)
    {
        int required_component_type_id = *(int*)array_get(&system->required_components, i);
        if(_entity_get_component(entity, required_component_type_id) == NULL)
        {
            return false;
        }
    }

    array_add(&system->entities, &handle);

    return true;
}

static void _system_component_added_event_callback(struct Event* sender, void* system, void* handle)
{
    _system_check_entity_add((struct _System*)system, *(EntityHandle*)handle);
}

static void _system_component_removed_event_callback(struct Event* sender, void* system, void* handle)
{
    struct _System* _system = system;
    EntityHandle _handle = *(EntityHandle*)handle;

    int entity_idx = array_find(&_system->entities, &_handle, &_compare_entity_handle);
    if(entity_idx == -1)
    {
        return;
    }

    array_remove_at(&_system->entities, entity_idx);
}

static bool _system_compare(const void* lhs, const void* rhs)
{
    const struct _System* lhs_sys = lhs;
    const struct _System* rhs_sys = rhs;

    return string_hash(&lhs_sys->name) == string_hash(&rhs_sys->name);
}

static void _system_alloc(void* new_system, void* new_system_args)
{
    struct _System* _new_system  = new_system;
    struct _SystemNewArgs* _args = new_system_args;

    string_init(&_new_system->name, _args->name->buffer);
    _new_system->update_func = _args->update_func;

    array_init(&_new_system->entities, sizeof(EntityHandle), DEFAULT_SYSTEM_ENTITIES_MAX, NULL, NULL);
    array_init(&_new_system->required_components, sizeof(int), DEFAULT_SYSTEM_REQUIRED_COMPONENTS_MAX, NULL, NULL);

    // Go through required components and set up event callbacks
    for(int i = 0; i < array_count(_args->required_components); ++i)
    {
        const struct string* component_name = array_get(_args->required_components, i);
        int hashed_name = string_hash(component_name);
        array_add(&_new_system->required_components, &hashed_name);

        struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, hashed_name);
        event_register_observer(&comp_cache->component_added_event, _new_system, &_system_component_added_event_callback);
        event_register_observer(&comp_cache->component_removed_event, _new_system, &_system_component_removed_event_callback);
    }
}

static void _system_free(void* system)
{
    struct _System* _system = system;
    string_uninit(&_system->name);
    array_uninit(&_system->entities);

    for(int i = 0; i < array_count(&_system->required_components); ++i)
    {
        int component_type_id = *(int*)array_get(&_system->required_components, i);
        struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);
        event_deregister_observer(&comp_cache->component_added_event, _system, &_system_compare);
        event_deregister_observer(&comp_cache->component_removed_event, _system, &_system_compare);
    }

    array_uninit(&_system->required_components);
}

// EXTERNAL FUNCS

void ecs_init(void)
{
    cache_init(&_ecs.entities, sizeof(struct _Entity), ENTITIES_MAX, &_entity_alloc, &_entity_free);
    cache_map_init(&_ecs.systems, sizeof(struct _System), SYSTEMS_MAX, &_system_alloc, &_system_free);
    cache_map_init(&_ecs.component_caches, sizeof(struct _ComponentCache), COMPONENT_TYPES_MAX, &_component_cache_alloc, &_component_cache_free);

    event_init(&_ecs.entity_created_event);
    event_init(&_ecs.entity_destroyed_event);
}

void ecs_uninit(void)
{
    cache_uninit(&_ecs.entities);
    cache_map_uninit(&_ecs.systems);
    cache_map_uninit(&_ecs.component_caches);

    event_uninit(&_ecs.entity_created_event);
    event_uninit(&_ecs.entity_destroyed_event);
}

int ecs_entities_count(void)
{
    return cache_size(&_ecs.entities);
}

int ecs_component_types_count(void)
{
    return cache_map_count(&_ecs.component_caches);
}

int ecs_components_count(const struct string* name)
{
    int component_type_id = string_hash(name);
    struct _ComponentCache* comp_cache =  cache_map_get_hashed(&_ecs.component_caches, component_type_id);
    return cache_size(&comp_cache->components);
}

int ecs_systems_count(void)
{
    return cache_map_count(&_ecs.systems);
}

EntityHandle entity_create(void)
{
    EntityHandle handle = cache_emplace(&_ecs.entities, NULL);

    if(handle == C_NULL_CACHE_HANDLE)
    {
        // TODO: Log error
    }

    return handle;
}

void entity_destroy(EntityHandle id)
{
    struct _Entity* entity = cache_get(&_ecs.entities, id);

    if(!entity)
    {
        return;
    }

    event_send(&_ecs.entity_destroyed_event, &id);

    for(int i = 0; i < array_count(&entity->components); ++i)
    {

        struct _Component* component = array_get(&entity->components, i);
        struct _ComponentCache* component_cache = cache_map_get_hashed(&_ecs.component_caches, component->type_id);
        _entity_remove_component(id, entity, component_cache, component, i);
    }

    cache_remove(&_ecs.entities, id);
}

void* entity_add_component(EntityHandle entity_handle, const struct string* component_name)
{
    int component_type_id = string_hash(component_name);
    struct _Entity* entity = cache_get(&_ecs.entities, entity_handle);
    struct _ComponentCache* component_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);

    if(component_cache == NULL || entity == NULL)
    {
        // TODO: Log debug error
        return NULL;
    }

    // Cannot add a duplicate component type, must be unique
    for(int i = 0; i < array_count(&entity->components); ++i)
    {
        struct _Component* check_this = array_get(&entity->components, i);
        if(check_this->type_id == component_type_id)
        {
            return NULL;
        }
    }

    struct _Component new_component;
    new_component.type_id = component_type_id;
    new_component.id = cache_emplace(&component_cache->components, NULL);

    array_add(&entity->components, &new_component);
    event_send(&component_cache->component_added_event, &entity_handle);
    return cache_get(&component_cache->components, new_component.id);
}

void entity_remove_component(EntityHandle entity_handle, const struct string* component_name)
{
    int component_type_id = string_hash(component_name);
    struct _Entity* entity = cache_get(&_ecs.entities, entity_handle);
    struct _ComponentCache* component_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);

    if(entity == NULL || component_cache == NULL)
    {
        // TODO: Log debug error
        return;
    }

    for(int i = 0; i < array_count(&entity->components); ++i)
    {
        struct _Component* check_this = array_get(&entity->components, i);
        if(check_this->type_id == component_type_id)
        {
            _entity_remove_component(entity_handle, entity, component_cache, check_this, i);
            return;
        }
    }
}

void* entity_get_component(EntityHandle entity_id, int component_type_id)
{
    struct _Entity* entity = cache_get(&_ecs.entities, entity_id);
    if(entity == NULL)
    {
        return NULL;
    }

    return _entity_get_component(entity, component_type_id);
}

void* entity_get_component_by_name(EntityHandle entity_id, const struct string* component_name)
{
    int component_type_id = string_hash(component_name);
    return entity_get_component(entity_id, component_type_id);
}

int entity_component_count(EntityHandle id)
{
    struct _Entity* entity = cache_get(&_ecs.entities, id);
    if(entity == NULL)
    {
        return -1;
    }

    return array_count(&entity->components);
}

void component_type_register(const struct string* name, int component_type_size_bytes)
{
    int component_type_id = string_hash(name);
    if(cache_map_get_hashed(&_ecs.component_caches, component_type_id))
    {
        // TODO: Log warning
        return;
    }

    struct _ComponentCacheNewArgs args;
    args.component_type_id = string_hash(name);
    args.item_size = component_type_size_bytes;
    args.capacity = 1024;
    args.alloc_func = NULL;
    args.free_func = NULL;

    cache_map_emplace(&_ecs.component_caches, name->buffer, name->size, &args);
}

void system_register(const struct string* name, system_update_fn update_func, const struct Array* required_components)
{
    int system_type_id = string_hash(name);
    if(cache_map_get_hashed(&_ecs.systems, system_type_id) != NULL)
    {
        // TODO: Log error
        return;
    }

    struct _SystemNewArgs args;
    args.name = name;
    args.update_func = update_func;
    args.required_components = required_components;

    cache_map_emplace(&_ecs.systems, name->buffer, name->size, &args);
}

void systems_update(void)
{
    struct It it = cache_map_begin(&_ecs.systems);
    struct It eit = cache_map_end(&_ecs.systems);
    for(; !it_eq(&it, &eit); cache_map_it_next(&it))
    {
        struct _System* sys = cache_map_it_get(&it);

        for(int i = 0; i < array_count(&sys->entities); ++i)
        {
            sys->update_func(*(EntityHandle*)array_get(&sys->entities, i));
        }
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

    return array_count(&system->entities);
}
