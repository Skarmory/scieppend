#ifndef SCIEPPEND_CORE_SYSTEM_H
#define SCIEPPNED_CORE_SYSTEM_H

#include "scieppend/core/array.h"
#include "scieppend/core/array_threadsafe.h"
#include "scieppend/core/ecs_defs.h"
#include "scieppend/core/event_defs.h"
#include "scieppend/core/string.h"

struct ECSWorld;
struct string;

struct SystemInitArgs
{
    struct ECSWorld* world;
    const struct string* name;
    const struct Array* required_components;
    SystemUpdateFn update_func;
};

struct System
{
    struct string name;
    struct ECSWorld* world;
    SystemUpdateFn update_func;
    ObserverHandle observer_handle;
    struct Array required_components;
    struct Array_ThreadSafe entity_handles;
};

struct System* system_new(struct ECSWorld* world, const struct string* name, const struct Array* required_components, SystemUpdateFn update_func);
void system_free(struct System* system);
void system_init(struct System* system, struct ECSWorld* world, const struct string* name, const struct Array* required_components, SystemUpdateFn update_func);
void system_init_wrapper(void* system, const void* args);
void system_uninit(struct System* system);
void system_uninit_wrapper(void* system);

// Accessors
int system_entities_count(const struct System* system);

// Mutators
void system_update(struct System* system);

#endif
