#ifndef SCIEPPEND_CORE_OBSERVER_H
#define SCIEPPEND_CORE_OBSERVER_H

#include "scieppend/core/array.h"
#include "scieppend/core/container_common.h"

struct Event;

typedef void(*event_callback_fn)(struct Event* sender, void* obs_data, void* event_args);

struct Observer
{
    void*             observer_data;
    event_callback_fn callback_func;
};

struct Event
{
    struct Array observers;
};

void event_init(struct Event* event);
void event_uninit(struct Event* event);
void event_register_observer(struct Event* event, void* observer_data, event_callback_fn callback_func);
void event_deregister_observer(struct Event* event, void* observer_data, compare_fn comp_func);
void event_send(struct Event* event, void* event_args);

#endif
