#include "scieppend/core/observer.h"

#include <stddef.h>

//static bool _compare_observer(void* lhs, void* rhs)
//{
//    return lhs == rhs;
//}
//
//void event_init(struct Event* event)
//{
//    array_init(&event->observers, sizeof(struct Observer), 8, NULL, NULL);
//}
//
//void event_uninit(struct Event* event)
//{
//    array_uninit(&event->observers);
//}
//
//void event_register_observer(struct Event* event, void* observer_data, event_callback_function callback_func)
//{
//    struct Observer new_obs;
//    new_obs.observer_data = observer_data;
//    new_obs.callback_func = callback_func;
//    array_add(&event->observers, &new_obs);
//}
//
//void event_deregister_observer(struct Event* event, void* observer_data)
//{
//    int idx = array_find(&event->observers, observer_data, &_compare_observer);
//
//    if(idx != -1)
//    {
//        array_remove_at(&event->observers, idx);
//    }
//}
//
//void event_send(struct Event* event, void* event_args)
//{
//    for(int i = 0; i < array_count(&event->observers); ++i)
//    {
//        struct Observer* obs = array_get(&event->observers, i);
//        obs->callback_func(obs->observer_data, event_args);
//    }
//}
