#ifndef SCIEPPEND_CORE_ENTITY_H
#define SCIEPPEND_CORE_ENTITY_H

#include "scieppend/core/array_threadsafe.h"
#include "scieppend/core/ecs_defs.h"

struct Array;

struct Entity
{
    struct Array_ThreadSafe components;
};

struct ComponentLookup
{
    ComponentHandle component_handle;
    ComponentTypeHandle component_type_handle;
};

void entity_init(struct Entity* entity);
void entity_uninit(struct Entity* entity);
void entity_init_wrapper(void* entity, const void* args);
void entity_uninit_wrapper(void* entity);

int entity_components_count(const struct Entity* entity);
bool entity_has_component(const struct Entity* entity, const ComponentTypeHandle component_type_handle);
bool entity_has_components(const struct Entity* entity, const struct Array* component_type_handles);
ComponentHandle entity_get_component(const struct Entity* entity, const ComponentTypeHandle component_type_handle);
const struct Array* entity_get_components(const struct Entity* entity);

void entity_add_component(struct Entity* entity, const ComponentHandle component_handle, const ComponentTypeHandle component_type_handle);
void entity_remove_component(struct Entity* entity, const ComponentTypeHandle component_type_hanle);
bool entity_lock(struct Entity* entity, bool write);
void entity_unlock(struct Entity* entity, bool write);

#endif
