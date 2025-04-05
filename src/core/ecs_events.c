#include "scieppend/core/ecs_events.h"

#include "scieppend/core/event.h"

enum ECSEventType component_event_get_event_type(struct ComponentEventArgs* args)
{
    return args->base.base.event_type;
}

enum ECSEventType entity_event_get_event_type(struct EntityEventArgs* args)
{
    return args->base.event_type;
}

void ecs_event_send_entity_event(const struct Event* event, enum ECSEventType event_type, EntityHandle entity_handle)
{
    struct EntityEventArgs event_args;
    event_args.base.event_type = event_type;
    event_args.entity_handle = entity_handle;

    event_send(event, &event_args);
}

void ecs_event_send_component_event(const struct Event* event, enum ECSEventType event_type, EntityHandle entity_handle, ComponentTypeHandle component_type_handle, ComponentHandle component_handle)
{
    struct ComponentEventArgs event_args;
    event_args.base.base.event_type = event_type;
    event_args.base.entity_handle = entity_handle;
    event_args.component_type = component_type_handle;
    event_args.component_handle = component_handle;

    event_send(event, &event_args);
}
