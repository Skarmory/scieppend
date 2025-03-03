#ifndef SCIEPPEND_CORE_ECS_DEFS_H
#define SCIEPPEND_CORE_ECS_DEFS_H

struct ECSWorld;

typedef int ComponentTypeHandle;
typedef int ComponentHandle;
typedef int EntityHandle;

typedef void(*SystemUpdateFn)(struct ECSWorld* world, EntityHandle handle);

extern const int C_NULL_COMPONENT_TYPE;
extern const int C_NULL_SYSTEM_TYPE;
extern const int C_NULL_ENTITY_HANDLE;
extern const int C_NULL_COMPONENT_HANDLE;

enum ECSEventType
{
    EVENT_COMPONENT_ADDED,
    EVENT_COMPONENT_REMOVED,
    EVENT_ENTITY_CREATED,
    EVENT_ENTITY_DESTROYED
};

#endif

