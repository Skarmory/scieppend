#include "scieppend/core/ecs.h"

#include "scieppend/core/cache.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

// STRUCTS

struct _Component
{
    ComponentHandle     id;
    ComponentTypeHandle type_id;
};

struct _System
{
    struct System          external;
    system_update_function update_func;
};

struct ComponentCacheNewArgs
{
    int item_size;
    int capacity;
    alloc_function alloc_func;
    free_function  free_func;
};

struct SystemNewArgs
{
    system_update_function update_func;
};

static struct ECS
{
    struct Cache* entities;
    struct Cache* component_caches;
    struct Cache* systems;
} _ecs;

// INTERNAL FUNCS

static void _component_cache_alloc(void* new_cache_vp, void* new_cache_args_vp)
{
    struct Cache* new_cache = new_cache_vp;
    struct ComponentCacheNewArgs* new_cache_args = new_cache_args_vp;
    cache_init(new_cache, new_cache_args->item_size, new_cache_args->capacity, new_cache_args->alloc_func, new_cache_args->free_func);
}

static void _component_cache_free(void* cache_vp)
{
    struct Cache* cache = cache_vp;
    cache_uninit(cache);
}

static void _system_alloc(void* new_system_vp, void* new_system_args_vp)
{
    struct _System* new_system = new_system_vp;
    struct SystemNewArgs* args = new_system_args_vp;
    new_system->update_func        = args->update_func;
    new_system->external.entities  = array_new(sizeof(EntityHandle), 32, NULL);
}

static void _system_free(void* system_vp)
{
    struct _System* system = system_vp;
    array_free(system->external.entities);
}

// EXTERNAL FUNCS

void ecs_init(void)
{
    _ecs.entities = cache_new(sizeof(struct Entity), 1024, NULL, NULL);
    _ecs.component_caches = cache_new(sizeof(struct Cache), 1024, _component_cache_alloc, _component_cache_free);
    _ecs.systems = cache_new(sizeof(struct _System), 1024, _system_alloc, _system_free);
}

void ecs_uninit(void)
{
    cache_free(_ecs.entities);
    cache_free(_ecs.component_caches);
    cache_free(_ecs.systems);
}

EntityHandle entity_create(void)
{
    int handle = cache_emplace(_ecs.entities, NULL);
    struct Entity* entity = cache_get(_ecs.entities, handle);
    entity->id = handle;
    entity->components = array_new(sizeof(struct _Component), 8, NULL);
    return handle;
}

void entity_destroy(EntityHandle id)
{
    struct Entity* entity = cache_get(_ecs.entities, id);

     //TODO notify systems here

    for(int i = 0; i < array_count(entity->components); ++i)
    {
        struct _Component* component = array_get(entity->components, i);
        struct Cache* component_cache = cache_get(_ecs.component_caches, component->type_id);
        cache_remove(component_cache, component->id);
    }

    array_free(entity->components);
    cache_remove(_ecs.entities, id);
}

void entity_add_component(EntityHandle id, ComponentTypeHandle component_type_id)
{
    struct Entity* entity = cache_get(_ecs.entities, id);
    for(int i = 0; i < array_count(entity->components); ++i)
    {
        struct _Component* icomp = array_get(entity->components, i);
        if(icomp->type_id == component_type_id)
        {
            // Cannot add a duplicate component type, must be unique
            return;
        }
    }

    struct Cache* component_cache = cache_get(_ecs.component_caches, component_type_id);

    struct _Component new_component;
    new_component.type_id = component_type_id;
    new_component.id = cache_emplace(component_cache, NULL);

    array_add(entity->components, &new_component);
}

void entity_remove_component(EntityHandle id, ComponentTypeHandle component_type_id)
{
    struct Entity* entity = cache_get(_ecs.entities, id);

    for(int i = 0; i < array_count(entity->components); ++i)
    {
        struct _Component* check_this = array_get(entity->components, i);
        if(check_this->type_id == component_type_id)
        {
            struct Cache* component_cache = cache_get(_ecs.component_caches, component_type_id);
            cache_remove(component_cache, check_this->id);
            array_remove_at(entity->components, i);
        }
    }
}

ComponentTypeHandle component_type_register(int size_bytes)
{
    struct ComponentCacheNewArgs args;
    args.item_size = size_bytes;
    args.capacity = 1024;
    args.alloc_func = NULL;
    args.free_func = NULL;

    int type_id = cache_emplace(_ecs.component_caches, &args);
    return type_id;
}

void system_register(system_update_function update_func)
{
    struct SystemNewArgs args;
    args.update_func = update_func;
    cache_emplace(_ecs.systems, &args);
}

void systems_update(void)
{
    for(struct CacheIt it = cache_begin(_ecs.systems); !cache_it_eq(it, cache_end(_ecs.systems)); it = cache_it_next(it))
    {
        struct _System* isys = cache_it_get(it);
        isys->update_func(&isys->external);
    }
}
