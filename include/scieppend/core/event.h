#ifndef SCIEPPEND_CORE_OBSERVER_H
#define SCIEPPEND_CORE_OBSERVER_H

#include "scieppend/core/array_threadsafe.h"
#include "scieppend/core/event_defs.h"

struct Event
{
    struct Array_ThreadSafe observers;
};

ObserverHandle observer_create(void* observer_data, event_callback_fn callback_func);
void observer_destroy(ObserverHandle handle);

struct Event* event_new(void);
void event_free(struct Event* event);
void event_init(struct Event* event);
void event_uninit(struct Event* event);
int  event_observer_count(struct Event* event);
void event_register_observer(struct Event* event, ObserverHandle obs_handle);
void event_deregister_observer(struct Event* event, ObserverHandle obs_handle);
void event_send(struct Event* event, void* event_args);

void eventing_init(void);
void eventing_uninit(void);

#endif
