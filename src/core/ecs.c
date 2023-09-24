#include "scieppend/core/ecs.h"

#include "scieppend/core/cache.h"
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
#define DEFAULT_ENTITY_COMPONENTS_MAX 8

// STRUCTS
//
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
    struct Cache components;
};

struct _System
{
    struct string          name;
    struct Array           entities;
    system_update_function update_func;
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
    const struct string*   name;
    system_update_function update_func;
};

static struct ECS
{
    struct Cache    entities;
    struct Cache    systems;
    struct CacheMap component_caches;
} _ecs;

// INTERNAL FUNCS

// Initialise a ComponentCache internals
static void _component_cache_alloc(void* new_comp_cache, void* new_cache_args)
{
    struct _ComponentCache* _new_comp_cache = new_comp_cache;
    struct _ComponentCacheNewArgs* _new_cache_args = new_cache_args;

    _new_comp_cache->component_type_id = _new_cache_args->component_type_id;

    cache_init(
        &_new_comp_cache->components,
        _new_cache_args->item_size,
        _new_cache_args->capacity,
        _new_cache_args->alloc_func,
        _new_cache_args->free_func
    );
}

// Uninitialise a ComponentCache internals
static void _component_cache_free(void* comp_cache)
{
    struct _ComponentCache* _comp_cache = comp_cache;
    cache_uninit(&_comp_cache->components);
}

static void _system_alloc(void* new_system, void* new_system_args)
{
    struct _System* _new_system  = new_system;
    struct _SystemNewArgs* _args = new_system_args;

    string_init(&_new_system->name, _args->name->buffer);
    _new_system->update_func = _args->update_func;

    array_init(&_new_system->entities, sizeof(EntityHandle), DEFAULT_SYSTEM_ENTITIES_MAX, NULL, NULL);
}

static void _system_free(void* system_vp)
{
    struct _System* system = system_vp;
    string_uninit(&system->name);
    array_uninit(&system->entities);
}

// EXTERNAL FUNCS

void ecs_init(void)
{
    cache_init(&_ecs.entities, sizeof(struct _Entity), ENTITIES_MAX, NULL, NULL);
    cache_init(&_ecs.systems, sizeof(struct _System), SYSTEMS_MAX, &_system_alloc, &_system_free);
    cache_map_init(&_ecs.component_caches, sizeof(struct _ComponentCache), COMPONENT_TYPES_MAX, &_component_cache_alloc, &_component_cache_free);
}

void ecs_uninit(void)
{
    cache_free(&_ecs.entities);
    cache_free(&_ecs.systems);
    cache_map_free(&_ecs.component_caches);
}

EntityHandle entity_create(void)
{
    int handle = cache_emplace(&_ecs.entities, NULL);
    struct _Entity* entity = cache_get(&_ecs.entities, handle);
    //entity->id = handle;
    array_init(&entity->components, sizeof(struct _Component), DEFAULT_ENTITY_COMPONENTS_MAX, NULL, NULL);
    return handle;
}

void entity_destroy(EntityHandle id)
{
    struct _Entity* entity = cache_get(&_ecs.entities, id);

     //TODO notify systems here

    for(int i = 0; i < array_count(&entity->components); ++i)
    {
        struct _Component* component = array_get(&entity->components, i);
        struct _ComponentCache* component_cache = cache_map_get_hashed(&_ecs.component_caches, component->type_id);
        cache_remove(&component_cache->components, component->id);
    }

    array_uninit(&entity->components);
    cache_remove(&_ecs.entities, id);
}

void entity_add_component(EntityHandle id, const struct string* component_name)
{
    int component_type_id = string_hash(component_name);
    struct _Entity* entity = cache_get(&_ecs.entities, id);
    struct _ComponentCache* component_cache = cache_map_get(&_ecs.component_caches, component_name->buffer, component_name->size);

    if(component_cache == NULL || entity == NULL)
    {
        // TODO: Log debug error
        return;
    }

    // Cannot add a duplicate component type, must be unique
    for(int i = 0; i < array_count(&entity->components); ++i)
    {
        struct _Component* check_this = array_get(&entity->components, i);
        if(check_this->type_id == component_type_id)
        {
            return;
        }
    }

    struct _Component new_component;
    new_component.type_id = component_type_id;
    new_component.id = cache_emplace(&component_cache->components, NULL);

    array_add(&entity->components, &new_component);
}

void entity_remove_component(EntityHandle id, const struct string* component_name)
{
    int component_type_id = string_hash(component_name);
    struct _Entity* entity = cache_get(&_ecs.entities, id);
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
            cache_remove(&component_cache->components, check_this->id);
            array_remove_at(&entity->components, i);
        }
    }
}

void* entity_get_component(EntityHandle id, const struct string* component_name)
{
    int component_type_id = string_hash(component_name);
    struct _Entity* entity = cache_get(&_ecs.entities, id);
    struct _ComponentCache* comp_cache = cache_map_get_hashed(&_ecs.component_caches, component_type_id);

    if(entity == NULL || comp_cache == NULL)
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

void component_type_register(const struct string* name, int component_type_size_bytes)
{
    struct _ComponentCacheNewArgs args;
    args.component_type_id = string_hash(name);
    args.item_size = component_type_size_bytes;
    args.capacity = 1024;
    args.alloc_func = NULL;
    args.free_func = NULL;

    cache_map_emplace(&_ecs.component_caches, name->buffer, name->size, &args);
}

void system_register(const struct string* name, system_update_function update_func)
{
    struct _SystemNewArgs args;
    args.name = name;
    args.update_func = update_func;
    cache_emplace(&_ecs.systems, &args);
}

void systems_update(void)
{
    for(struct CacheIt it = cache_begin(&_ecs.systems); !cache_it_eq(it, cache_end(&_ecs.systems)); it = cache_it_next(it))
    {
        struct _System* sys = cache_it_get(it);

        for(int i = 0; i < array_count(&sys->entities); ++i)
        {
            sys->update_func(*(EntityHandle*)array_get(&sys->entities, i));
        }
    }
}
