#include "scieppend/test/core/event.h"

#include "scieppend/core/container_common.h"
#include "scieppend/core/event.h"
#include "scieppend/test/test.h"

#include <stddef.h>

struct EventTestState
{
    struct Event event;
    struct Array observers;
};

struct ObserverTestData
{
    int x;
    int y;
};

static void _observer_callback([[maybe_unused]] struct Event* sender, void* data, [[maybe_unused]] void* event_args)
{
    struct ObserverTestData* _data = data;
    ++(_data->x);
    --(_data->y);

    int* event_arg = event_args;
    ++(*event_arg);
}

static int _observer_compare(const void* lhs, const void* rhs)
{
    const struct ObserverTestData* lhs_obs = lhs;
    const struct ObserverTestData* rhs_obs = rhs;
    return lhs_obs->x == rhs_obs->x && lhs_obs->y == rhs_obs->y;
}

static void _setup_event(void* userstate)
{
    struct EventTestState* state = userstate;
    event_init(&state->event);
}

static void _teardown_event(void* userstate)
{
    struct EventTestState* state = userstate;
    event_uninit(&state->event);
}

static void _setup_observer(void* userstate)
{
    struct EventTestState* state = userstate;
    array_init(&state->observers, sizeof(struct ObserverTestData), 8, NULL, NULL);
    for(int i = 0; i < 8; ++i)
    {
        struct ObserverTestData* obs_data = array_emplace(&state->observers, NULL);
        obs_data->x = i;
        obs_data->y = 8-i;
    }
}

static void _teardown_observer(void* userstate)
{
    struct EventTestState* state = userstate;
    array_uninit(&state->observers);
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
    test_assert_equal_int("event observer count", 0, array_count(&state->event.observers));
    event_register_observer(&state->event, array_get(&state->observers, 0), &_observer_callback);
    test_assert_equal_int("event observer count", 1, array_count(&state->event.observers));
    event_deregister_observer(&state->event, array_get(&state->observers, 0), &_observer_compare);
    test_assert_equal_int("event observer count", 0, array_count(&state->event.observers));
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
    for(int i = 0; i < array_count(&state->observers); ++i)
    {
        event_register_observer(&state->event, array_get(&state->observers, i), &_observer_callback);
    }
}

void _teardown_send(void* userstate)
{
    struct EventTestState* state = userstate;
    for(int i = array_count(&state->observers) - 1; i > -1; --i)
    {
        event_deregister_observer(&state->event, array_get(&state->observers, i), &_observer_compare);
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

    for(int i = 0; i < array_count(&state->observers); ++i)
    {
        test_assert_item_in_array("observer data", expect, sizeof(struct ObserverTestData), 8, array_get(&state->observers, i), &_observer_compare);
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
