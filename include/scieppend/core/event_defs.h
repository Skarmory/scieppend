#ifndef SCIEPPEND_CORE_EVENT_DEFS_H
#define SCIEPPEND_CORE_EVENT_DEFS_H

struct Event;

typedef void(*event_callback_fn)(const struct Event* sender, void* obs_data, void* event_args);
typedef int ObserverHandle;

#endif
