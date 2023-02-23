#ifndef SCIEPPEND_CORE_ENTITY_H
#define SCIEPPEND_CORE_ENTITY_H

#define ENTITY_MAX_COMPONENTS 256

#include "scieppend/core/array.h"
#include "scieppend/core/stack_array.h"

typedef int EntityHandle;
typedef int ComponentHandle;
typedef int ComponentTypeHandle;

struct Entity
{
    EntityHandle id;
    struct Array* components;
};

struct System
{
    struct Array* entities;
};

typedef void(*system_update_function)(struct System*);

/* Initialise the internal ECS containers.
 */
void ecs_init(void);

/* Uninitialise the internal ECS containers.
 */
void ecs_uninit(void);

/* Creates an entity and returns a handle to it.
 */
EntityHandle entity_create(void);

/* Destroys an entity, removes all its components, and notifies all interested systems.
 */
void entity_destroy(EntityHandle id);

/* Adds a new component of the given type to the entity of given id.
 */
void entity_add_component(EntityHandle id, ComponentTypeHandle component_type_id);

/* Removes the given component type from an entity.
 */
void entity_remove_component(EntityHandle id, ComponentTypeHandle component_type_id);
//void entity_remove_component(EntityHandle id, struct Component* component);

/* Creates a component cache for a particular component type.
 * Returns the id for a component type.
 */
ComponentTypeHandle component_type_register(int size_bytes);

/* Internally creates a system that will call the given function every update cycle.
 */
void system_register(system_update_function update_func);

/* Iterate over all registered systems and calls their update functions.
 */
void systems_update(void);

#endif

