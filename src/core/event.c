#include "scieppend/core/event.h"

#include <stddef.h>

void event_init(struct Event* event)
{
    array_init(&event->observers, sizeof(struct Observer), 8, NULL, NULL);
}

void event_uninit(struct Event* event)
{
    array_uninit(&event->observers);
}

void event_register_observer(struct Event* event, void* observer_data, event_callback_fn callback_func)
{
    struct Observer new_obs;
    new_obs.observer_data = observer_data;
    new_obs.callback_func = callback_func;
    array_add(&event->observers, &new_obs);
}

void event_deregister_observer(struct Event* event, void* observer_data, compare_fn comp_func)
{
    for(int i = 0; i < array_count(&event->observers); ++i)
    {
        const struct Observer* observer = array_get(&event->observers, i);
        if(comp_func(observer->observer_data, observer_data))
        {
            array_remove_at(&event->observers, i);
            return;
        }
    }
}

void event_send(struct Event* event, void* event_args)
{
    for(int i = 0; i < array_count(&event->observers); ++i)
    {
        struct Observer* obs = array_get(&event->observers, i);
        obs->callback_func(event, obs->observer_data, event_args);
    }
}
