#ifndef SCIEPPEND_CORE_ECS_WORLD_H
#define SCIEPPEND_CORE_ECS_WORLD_H

#include "scieppend/core/ecs_defs.h"
#include "scieppend/core/event.h"

#include <stdbool.h>

struct Array;
struct ECSWorld;
struct string;

struct ECSWorld* ecs_world_new(void);
void ecs_world_free(struct ECSWorld* world);

// World functions
int ecs_world_entities_count(const struct ECSWorld* world);
int ecs_world_systems_count(const struct ECSWorld* world);
int ecs_world_component_types_count(const struct ECSWorld* world);
int ecs_world_components_count(const struct ECSWorld* world, ComponentTypeHandle handle);

// Entity functions
EntityHandle ecs_world_create_entity(struct ECSWorld* world);
void ecs_world_destroy_entity(struct ECSWorld* world, EntityHandle entity_handle);

// Component functions
void ecs_world_entity_add_component(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle);
void ecs_world_entity_remove_component(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle);
void* ecs_world_entity_get_component(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle, bool write);
void ecs_world_entity_unget_component(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle, bool write);
bool ecs_world_entity_has_component(struct ECSWorld* world, EntityHandle entity_handle, const ComponentTypeHandle component_type_handle);
bool ecs_world_entity_has_components(struct ECSWorld* world, EntityHandle entity_handle, const struct Array* component_type_handles);
int ecs_world_entity_components_count(struct ECSWorld* world, EntityHandle entity_handle);
ComponentHandle ecs_world_entity_get_component_handle(struct ECSWorld* world, const EntityHandle entity_handle, const ComponentTypeHandle component_type_handle);

// Component type functions
void ecs_world_component_type_register(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, int bytes);
void ecs_world_component_type_lock(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, bool write);
void ecs_world_component_type_unlock(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, bool write);
void ecs_world_component_type_register_observer(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, const ObserverHandle observer);
void ecs_world_component_type_deregister_observer(struct ECSWorld* world, const ComponentTypeHandle component_type_handle, const ObserverHandle observer);

// System functions
void ecs_world_system_register(struct ECSWorld* world, const struct string* system_name, const struct Array* required_components, SystemUpdateFn update_func);
struct System* ecs_world_get_system(const struct ECSWorld* world, const struct string* system_name);
void ecs_world_update_systems(const struct ECSWorld* world);
int ecs_world_system_entities_count(const struct ECSWorld* world, const struct string* system_name);

#endif
