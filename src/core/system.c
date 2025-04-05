#include "scieppend/core/system.h"

#include "scieppend/core/ecs_events.h"
#include "scieppend/core/ecs_world.h"
#include "scieppend/core/event.h"

#include <stdlib.h>

#define DEFAULT_ENTITIES_CAPACITY 64

// Pre-computed hash of "__NullSystemType" string
const int C_NULL_SYSTEM_TYPE = -764810385;

enum _EntityOperation
{
    ENTITY_OP_ADD,
    ENTITY_OP_REMOVE
};

struct _EntityCommand
{
    EntityHandle handle;
    enum _EntityOperation op;
};

// ---------- INTERNAL FUNCS ----------

static int _compare_entity_handle(const void* lhs, const void* rhs)
{
    const EntityHandle* _lhs = lhs;
    const EntityHandle* _rhs = rhs;

    if(*_lhs == *_rhs)
    {
        return 1;
    }

    return 0;
}

static void _system_add_entity(struct System* system, EntityHandle entity_handle)
{
    if(ecs_world_entity_has_components(system->world, entity_handle, &system->required_components))
    {
        if (system->state == SYSTEM_STATE_UPDATING)
        {
            struct _EntityCommand cmd =
            {
                .handle = entity_handle,
                .op = ENTITY_OP_ADD
            };

            array_ts_add(&system->ecs_commands, &cmd);
        }
        else
        {
            array_ts_add(&system->entity_handles, &entity_handle);
        }
    }
}

static void _system_remove_entity(struct System* system, EntityHandle entity_handle)
{
    if (system->state == SYSTEM_STATE_UPDATING)
    {
        struct _EntityCommand cmd =
        {
            .handle = entity_handle,
            .op = ENTITY_OP_REMOVE
        };

        array_ts_add(&system->ecs_commands, &cmd);
    }
    else
    {
        array_ts_find_and_remove(&system->entity_handles, &entity_handle, &_compare_entity_handle);
    }
}

static void _system_event_callback([[maybe_unused]] const struct Event* sender, void* observer_data, void* event_args)
{
    struct System* system = observer_data;
    enum ECSEventType event_type = ((struct ECSEventArgs*)event_args)->event_type;

    switch(event_type)
    {
        case EVENT_COMPONENT_ADDED:
        case EVENT_ENTITY_CREATED:
            {
                struct EntityEventArgs* args = event_args;
                _system_add_entity(system, args->entity_handle);
            }
            break;
        case EVENT_COMPONENT_REMOVED:
        case EVENT_ENTITY_DESTROYED:
            {
                struct EntityEventArgs* args = event_args;
                _system_remove_entity(system, args->entity_handle);
            }
            break;
        default:
            break;
    }
}

// ---------- EXTERNAL FUNCS ----------

struct System* system_new(struct ECSWorld* world, const struct string* name, const struct Array* required_components, SystemUpdateFn update_func)
{
    struct System* new_system = malloc(sizeof(struct System));
    system_init(new_system, world, name, required_components, update_func);
    return new_system;
}

void system_init(struct System* system, struct ECSWorld* world, const struct string* name, const struct Array* required_components, SystemUpdateFn update_func)
{
    string_init(&system->name, name->buffer);
    system->state = SYSTEM_STATE_IDLE;
    system->world = world;
    system->update_func = update_func;
    system->observer_handle = observer_create(system, &_system_event_callback);
    array_init(&system->required_components, sizeof(ComponentTypeHandle), array_count(required_components), NULL, NULL);
    array_ts_init(&system->entity_handles, sizeof(EntityHandle), DEFAULT_ENTITIES_CAPACITY, NULL, NULL);
    array_ts_init(&system->ecs_commands, sizeof(struct _EntityCommand), 8, NULL, NULL);

    for(int i = 0; i < array_count(required_components); ++i)
    {
        ComponentTypeHandle component_type_handle = *(ComponentTypeHandle*)array_get(required_components, i);
        array_add(&system->required_components, &component_type_handle);
        ecs_world_component_type_register_observer(world, component_type_handle, system->observer_handle);
    }
}

void system_init_wrapper(void* system, const void* args)
{
    struct System* _system = system;
    const struct SystemInitArgs* _args = args;
    system_init(_system, _args->world, _args->name, _args->required_components, _args->update_func);
}


void system_free(struct System* system)
{
    system_uninit(system);
    free(system);
}

void system_uninit(struct System* system)
{
    for(int i = 0; i < array_count(&system->required_components); ++i)
    {
        ComponentTypeHandle component_type_handle = *(ComponentTypeHandle*)array_get(&system->required_components, i);
        ecs_world_component_type_deregister_observer(system->world, component_type_handle, system->observer_handle);
    }

    observer_destroy(system->observer_handle);
    array_ts_uninit(&system->ecs_commands);
    array_ts_uninit(&system->entity_handles);
    array_uninit(&system->required_components);
    string_uninit(&system->name);
}

void system_uninit_wrapper(void* system)
{
    struct System* _system = system;
    system_uninit(_system);
}

int system_entities_count(const struct System* system)
{
    return array_ts_count(&system->entity_handles);
}

void system_update(struct System* system)
{
    system->state = SYSTEM_STATE_UPDATING;
    array_ts_lock(&system->entity_handles, WRITE);

    // Process entities 
    for(int i = 0; i < array_count(&system->entity_handles.array); ++i)
    {
        EntityHandle entity_handle = *(EntityHandle*)array_get(&system->entity_handles.array, i);
        system->update_func(system->world, entity_handle);
    }

    array_ts_unlock(&system->entity_handles, WRITE);
    system->state = SYSTEM_STATE_IDLE;
}

void system_process_ecs_commands(struct System* system)
{
    array_ts_lock(&system->ecs_commands, WRITE);
    array_ts_lock(&system->entity_handles, WRITE);
    {
        for(int i = 0; i < array_count(&system->ecs_commands.array); ++i)
        {
            struct _EntityCommand* cmd = array_get(&system->ecs_commands.array, i);
            switch(cmd->op)
            {
                case ENTITY_OP_REMOVE:
                    array_find_and_remove(&system->entity_handles.array, &cmd->handle, &_compare_entity_handle);
                    break;
                case ENTITY_OP_ADD:
                    array_add(&system->entity_handles.array, &cmd->handle);
                    break;
            }
        }
    }
    array_ts_unlock(&system->entity_handles, WRITE);
    array_clear(&system->ecs_commands.array);
    array_ts_unlock(&system->ecs_commands, WRITE);
}
