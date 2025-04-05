#include "scieppend/core/event.h"

#include "scieppend/core/cache_threadsafe.h"
#include "scieppend/core/container_common.h"

#include <stddef.h>
#include <stdlib.h>

static struct _ObserverManager
{
    struct Cache_ThreadSafe observers;
} _obs_man;

struct Observer
{
    void*             observer_data;
    event_callback_fn callback_func;
};

// INTERNAL FUNCS

static int _compare_observer_handle(const void* lhs, const void* rhs)
{
    return *(const ObserverHandle*)lhs == *(const ObserverHandle*)rhs;
}

// EXTENRAL FUNCS

ObserverHandle observer_create(void* observer_data, event_callback_fn event_callback_func)
{
    struct Observer new_obs;
    new_obs.observer_data = observer_data;
    new_obs.callback_func = event_callback_func;
    return cache_ts_add(&_obs_man.observers, &new_obs);
}

void observer_destroy(ObserverHandle handle)
{
    cache_ts_remove(&_obs_man.observers, handle);
}

struct Event* event_new(void)
{
    struct Event* event = malloc(sizeof(struct Event));
    event_init(event);
    return event;
}

void event_free(struct Event* event)
{
    event_uninit(event);
    free(event);
}

void event_init(struct Event* event)
{
    array_ts_init(&event->observers, sizeof(ObserverHandle), 8, NULL, NULL);
}

void event_uninit(struct Event* event)
{
    array_ts_uninit(&event->observers);
}

int event_observer_count(const struct Event* event)
{
    return array_ts_count(&event->observers);
}

void event_register_observer(struct Event* event, ObserverHandle obs_handle)
{
    array_ts_add(&event->observers, &obs_handle);
}

void event_deregister_observer(struct Event* event, ObserverHandle obs_handle)
{
    int idx = array_ts_find(&event->observers, &obs_handle, &_compare_observer_handle);
    if(idx != -1)
    {
        array_ts_remove_at(&event->observers, idx);
    }
}

void event_send(const struct Event* event, void* event_args)
{
    for(int i = 0; i < array_ts_count(&event->observers); ++i)
    {
        ObserverHandle* obs_h = array_ts_get(&event->observers, i);
        struct Observer* obs = cache_ts_get(&_obs_man.observers, *obs_h);
        obs->callback_func(event, obs->observer_data, event_args);
    }
}

void eventing_init(void)
{
    cache_ts_init(&_obs_man.observers, sizeof(struct Observer), 64, NULL, NULL);
}

void eventing_uninit(void)
{
    cache_ts_uninit(&_obs_man.observers);
}
