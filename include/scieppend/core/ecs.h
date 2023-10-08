#ifndef SCIEPPEND_CORE_ENTITY_H
#define SCIEPPEND_CORE_ENTITY_H

#define ENTITY_MAX_COMPONENTS 256

#include "scieppend/core/array.h"
#include "scieppend/core/stack_array.h"

struct string;

typedef int EntityHandle;
typedef int ComponentHandle;
typedef int ComponentTypeHandle;
typedef void(*system_update_fn)(EntityHandle handle);

/* Initialise the internal ECS containers.
 */
void ecs_init(void);

/* Uninitialise the internal ECS containers.
 */
void ecs_uninit(void);

int ecs_entities_count(void);
int ecs_component_types_count(void);
int ecs_components_count(const struct string* name);
int ecs_systems_count(void);

/* Creates an entity and returns a handle to it.
 */
EntityHandle entity_create(void);

/* Destroys an entity, removes all its components, and notifies all interested systems.
 */
void entity_destroy(EntityHandle id);

/* Adds a new component of the given type to the entity of given id.
 */
void* entity_add_component(EntityHandle id, const struct string* component_name);

/* Removes the given component type from an entity.
 */
void entity_remove_component(EntityHandle id, const struct string* component_name);

/* Get a component from an entity.
 * Return null if no component found.
 */
void* entity_get_component(EntityHandle entity_id, int component_id);
void* entity_get_component_by_name(EntityHandle entity_id, const struct string* component_name);

int entity_component_count(EntityHandle id);

/* Creates a component cache for a particular component type.
 * Returns the id for a component type.
 */
void component_type_register(const struct string* name, int component_type_size_bytes);

/* Internally creates a system that will call the given function every update cycle.
 */
void system_register(const struct string* name, system_update_fn update_func, const struct Array* required_components);

/* Iterate over all registered systems and calls their update functions.
 */
void systems_update(void);

int system_entity_count(const struct string* system_name);

#endif

