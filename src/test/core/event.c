#include "scieppend/test/core/event.h"

#include "scieppend/core/array.h"
#include "scieppend/core/container_common.h"
#include "scieppend/core/event.h"
#include "scieppend/test/test.h"

#include <stddef.h>

struct EventTestState
{
    struct Event event;
    struct Array observers;
    struct Array observer_datas;
};

struct ObserverTestData
{
    int x;
    int y;
};

static int _observer_compare(const void* lhs, const void* rhs)
{
    const struct ObserverTestData* _lhs = lhs;
    const struct ObserverTestData* _rhs = rhs;
    return _lhs->x == _rhs->x && _lhs->y == _rhs->y;
}

static void _observer_callback([[maybe_unused]] struct Event* sender, void* data, [[maybe_unused]] void* event_args)
{
    struct ObserverTestData* _data = data;
    ++(_data->x);
    --(_data->y);

    int* event_arg = event_args;
    ++(*event_arg);
}

static void _setup_event(void* userstate)
{
    struct EventTestState* state = userstate;
    eventing_init();
    event_init(&state->event);
}

static void _teardown_event(void* userstate)
{
    struct EventTestState* state = userstate;
    event_uninit(&state->event);
    eventing_uninit();
}

static void _setup_observer(void* userstate)
{
    struct EventTestState* state = userstate;
    array_init(&state->observer_datas, sizeof(struct ObserverTestData), 8, NULL, NULL);
    array_init(&state->observers, sizeof(ObserverHandle), array_count(&state->observer_datas), NULL, NULL);
    for(int i = 0; i < 8; ++i)
    {
        struct ObserverTestData* obs_data = array_emplace(&state->observer_datas, NULL);
        obs_data->x = i;
        obs_data->y = 8-i;
    }
}

static void _teardown_observer(void* userstate)
{
    struct EventTestState* state = userstate;
    array_uninit(&state->observers);
    array_uninit(&state->observer_datas);
}

static void _setup(void* userstate)
{
    _setup_event(userstate);
    _setup_observer(userstate);
}

static void _teardown(void* userstate)
{
    _teardown_observer(userstate);
    _teardown_event(userstate);
}

static void _test_event_register_and_deregister_observer(void* userstate)
{
    struct EventTestState* state = userstate;

    ObserverHandle obs = observer_create(NULL, NULL);
    test_assert_equal_int("event observer count", 0, event_observer_count(&state->event));
    event_register_observer(&state->event, obs);
    test_assert_equal_int("event observer count", 1, event_observer_count(&state->event));
    event_deregister_observer(&state->event, obs);
    test_assert_equal_int("event observer count", 0, event_observer_count(&state->event));
    observer_destroy(obs);
}

void test_event_register_deregister(void)
{
    struct EventTestState state;
    testing_add_group("register observer");
    testing_add_test("event register and deregister", &_setup, &_teardown, &_test_event_register_and_deregister_observer, &state, sizeof(state));
}

void _setup_send(void* userstate)
{
    struct EventTestState* state = userstate;
    _setup(userstate);
    for(int i = 0; i < array_count(&state->observer_datas); ++i)
    {
        ObserverHandle observer = observer_create(array_get(&state->observer_datas, i), &_observer_callback);
        array_add(&state->observers, &observer);
        event_register_observer(&state->event, observer);
    }
}

void _teardown_send(void* userstate)
{
    struct EventTestState* state = userstate;
    for(int i = array_count(&state->observer_datas) - 1; i > -1; --i)
    {
        ObserverHandle* obs = array_get(&state->observers, i);
        event_deregister_observer(&state->event, *obs);
        observer_destroy(*obs);
    }
    _teardown(state);
}

void _test_event_send(void* userstate)
{
    struct EventTestState* state = userstate;
    int counter = 0;
    event_send(&state->event, &counter);
    test_assert_equal_int("event received count", array_count(&state->observers), counter);

    static const struct ObserverTestData expect[] = {
        { 1, 7 }, { 2, 6 }, { 3, 5 }, { 4, 4 },
        { 5, 3 }, { 6, 2 }, { 7, 1 }, { 8, 0 }
    };

    for(int i = 0; i < array_count(&state->observer_datas); ++i)
    {
        test_assert_item_in_array("observer data", expect, sizeof(struct ObserverTestData), 8, array_get(&state->observer_datas, i), &_observer_compare);
    }
}

void test_event_send(void)
{
    struct EventTestState state;
    testing_add_group("send");
    testing_add_test("event send", &_setup_send, &_teardown_send, &_test_event_send, &state, sizeof(state));
}

void test_event_run_all(void)
{
    test_event_register_deregister();
    test_event_send();
}
