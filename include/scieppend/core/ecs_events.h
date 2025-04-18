#ifndef SCIEPPEND_CORE_ECS_EVENTS_H
#define SCIEPPEND_CORE_ECS_EVENTS_H

#include "scieppend/core/ecs_defs.h"

struct Event;

struct ECSEventArgs
{
    enum ECSEventType event_type;
};

struct EntityEventArgs
{
    struct ECSEventArgs base;
    EntityHandle        entity_handle;
};

struct ComponentEventArgs
{
    struct EntityEventArgs base;
    ComponentTypeHandle    component_type;
    ComponentHandle        component_handle;
};

enum ECSEventType component_event_get_event_type(struct ComponentEventArgs* args);
enum ECSEventType entity_event_get_event_type(struct EntityEventArgs* args);

void ecs_event_send_entity_event(const struct Event* event, enum ECSEventType event_type, EntityHandle entity_handle);
void ecs_event_send_component_event(const struct Event* event, enum ECSEventType event_type, EntityHandle entity_handle, ComponentTypeHandle component_type_handle, ComponentHandle component_handle);

#endif

