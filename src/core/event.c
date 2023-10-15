#include "scieppend/core/event.h"

#include "scieppend/core/cache.h"
#include "scieppend/core/container_common.h"

#include <stddef.h>

static struct _ObserverManager
{
    struct Cache observers;
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
    return cache_add(&_obs_man.observers, &new_obs);
}

void observer_destroy(ObserverHandle handle)
{
    cache_remove(&_obs_man.observers, handle);
}

void event_init(struct Event* event)
{
    array_init(&event->observers, sizeof(ObserverHandle), 8, NULL, NULL);
}

void event_uninit(struct Event* event)
{
    array_uninit(&event->observers);
}

int event_observer_count(struct Event* event)
{
    return array_count(&event->observers);
}

void event_register_observer(struct Event* event, ObserverHandle obs_handle)
{
    array_add(&event->observers, &obs_handle);
}

void event_deregister_observer(struct Event* event, ObserverHandle obs_handle)
{
    int idx = array_find(&event->observers, &obs_handle, &_compare_observer_handle);
    if(idx != -1)
    {
        array_remove_at(&event->observers, idx);
    }
}

void event_send(struct Event* event, void* event_args)
{
    for(int i = 0; i < array_count(&event->observers); ++i)
    {
        ObserverHandle* obs_h = array_get(&event->observers, i);
        struct Observer* obs = cache_get(&_obs_man.observers, *obs_h);
        obs->callback_func(event, obs->observer_data, event_args);
    }
}

void eventing_init(void)
{
    cache_init(&_obs_man.observers, sizeof(struct Observer), 64, NULL, NULL);
}

void eventing_uninit(void)
{
    cache_uninit(&_obs_man.observers);
}
