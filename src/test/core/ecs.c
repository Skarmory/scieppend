#include "scieppend/test/core/ecs.h"

#include "scieppend/core/ecs.h"
#include "scieppend/test/test.h"

struct TestComponent
{
    int x;
    int y;
    int z;
};

struct SystemTestState
{
    int system_update_var1;
    int system_update_var2;
    int system_update_var3;
};

struct EntityTestState
{
    ComponentTypeHandle component_type_id;
};

static void _system_update_func1(struct System* sys)
{
    //++_test_system_update_var1;
}

static void _system_update_func2(struct System* sys)
{
    //++_test_system_update_var2;
}

static void _system_update_func3(struct System* sys)
{
    //++_test_system_update_var3;
}

static void _setup([[maybe_unused]] void* userstate)
{
    ecs_init();
}

static void _teardown([[maybe_unused]] void* userstate)
{
    ecs_uninit();
}

static void _system_setup(void* userstate)
{
    struct SystemTestState* state = userstate;

    _setup(userstate);

    state->system_update_var1 = 0;
    state->system_update_var2 = 0;
    state->system_update_var3 = 0;
}

static void _system_teardown([[maybe_unused]] void* userstate)
{
    _teardown(userstate);
}

static void _setup_entity([[maybe_unused]] void* userstate)
{
    struct EntityTestState* state = userstate;
    _setup(userstate);
    state->component_type_id = component_type_register(sizeof(struct TestComponent));
}

static void _teardown_entity([[maybe_unused]] void* userstate)
{
    _teardown(userstate);
}

static void _test_system_update([[maybe_unused]] void* userstate)
{
    struct SystemTestState* state = userstate;

    system_register(&_system_update_func1);
    system_register(&_system_update_func2);
    system_register(&_system_update_func3);

    systems_update();


    systems_update();

    test_assert_equal_int("system 1 post-update value", 2, state->system_update_var1);
    test_assert_equal_int("system 2 post-update value", 2, state->system_update_var2);
    test_assert_equal_int("system 3 post-update value", 2, state->system_update_var3);
}

static void _test_entity_create_destroy([[maybe_unused]] void* userstate)
{
    struct EntityTestState* state = userstate;

    EntityHandle entity = entity_create();
    entity_add_component(entity, state->component_type_id);
    entity_remove_component(entity, state->component_type_id);
    //entity_destroy(entity);
}

void test_ecs_systems(void)
{
    struct SystemTestState state;
    testing_add_group("system");
    testing_add_test("system update", &_setup, &_teardown, &_test_system_update, &state, sizeof(state));
}

void test_ecs_entity(void)
{
    testing_add_group("entity");
    testing_add_test("entity creation", &_setup_entity, &_teardown_entity, &_test_entity_create_destroy, NULL, 0);
}

void test_ecs_run_all(void)
{
    test_ecs_systems();
    test_ecs_entity();
}
