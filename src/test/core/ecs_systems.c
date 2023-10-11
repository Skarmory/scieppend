#include "scieppend/test/core/ecs.h"

#include "scieppend/core/ecs.h"
#include "scieppend/core/string.h"
#include "scieppend/test/core/ecs_common.h"
#include "scieppend/test/test.h"

struct SystemTestState
{
    struct string component_A_name;
    struct string component_B_name;
    struct string component_C_name;
    struct string sys_name;
    struct Array  required_components;
};

// INTERNAL FUNCS

static void _system_update(EntityHandle entity_handle)
{
    struct ECSTestComponentA* comp_a = entity_get_component(entity_handle, G_TEST_COMPONENT_A_ID);
    struct ECSTestComponentB* comp_b = entity_get_component(entity_handle, G_TEST_COMPONENT_B_ID);
    struct ECSTestComponentC* comp_c = entity_get_component(entity_handle, G_TEST_COMPONENT_C_ID);

    comp_a->x++;
    comp_a->y++;
    comp_a->z++;
}

static void _setup(void* userstate)
{
    struct SystemTestState* state = userstate;
    string_init(&state->sys_name, "TestSystem");
    string_init(&state->component_A_name, C_TEST_COMPONENT_A_NAME);
    string_init(&state->component_B_name, C_TEST_COMPONENT_B_NAME);
    string_init(&state->component_C_name, C_TEST_COMPONENT_C_NAME);
    array_init(&state->required_components, sizeof(int), 3, NULL, NULL);
    ecs_init();
}

static void _teardown(void* userstate)
{
    struct SystemTestState* state = userstate;
    ecs_uninit();
    array_uninit(&state->required_components);
    string_uninit(&state->component_C_name);
    string_uninit(&state->component_B_name);
    string_uninit(&state->component_A_name);
    string_uninit(&state->sys_name);
}

static void _setup__component_types(void* userstate)
{
    struct SystemTestState* state = userstate;

    _setup(userstate);

    component_type_register(&state->component_A_name, sizeof(struct ECSTestComponentA));
    component_type_register(&state->component_B_name, sizeof(struct ECSTestComponentB));
    component_type_register(&state->component_C_name, sizeof(struct ECSTestComponentC));

    G_TEST_COMPONENT_A_ID = string_hash(&state->component_A_name);
    G_TEST_COMPONENT_B_ID = string_hash(&state->component_B_name);
    G_TEST_COMPONENT_C_ID = string_hash(&state->component_C_name);
}

static void _teardown__component_types(void* userstate)
{
    struct SystemTestState* state = userstate;
    _teardown(userstate);
}

static void _setup__system(void* userstate)
{
    struct SystemTestState* state = userstate;

    _setup__component_types(userstate);

    array_add(&state->required_components, &G_TEST_COMPONENT_A_ID);
    array_add(&state->required_components, &G_TEST_COMPONENT_B_ID);
    array_add(&state->required_components, &G_TEST_COMPONENT_C_ID);

    system_register(&state->sys_name, &_system_update, &state->required_components);
}

static void _teardown__system(void* userstate)
{
    struct SystemTestState* state = userstate;
    _teardown__component_types(userstate);
}

// TESTS

void _test__system_register(void* userstate)
{
    struct SystemTestState* state = userstate;

    test_assert_equal_int("systems registered count", 0, ecs_systems_count());

    system_register(&state->sys_name, NULL, &state->required_components);

    test_assert_equal_int("systems registered count", 1, ecs_systems_count());

}

void _test__system_uniqueness(void* userstate)
{
    struct SystemTestState* state = userstate;

    test_assert_equal_int("systems registered count", 0, ecs_systems_count());

    system_register(&state->sys_name, NULL, &state->required_components);
    system_register(&state->sys_name, NULL, &state->required_components);
    system_register(&state->sys_name, NULL, &state->required_components);

    test_assert_equal_int("systems registered count", 1, ecs_systems_count());
}

void _test__system_entity_required_components_added(void* userstate)
{
    struct SystemTestState* state = userstate;

    EntityHandle handle = entity_create();

    int* component_type_id = array_get(&state->required_components, 0);
    entity_add_component(handle, *component_type_id);
    test_assert_equal_int("system has entity, 1/3 required components", 0, system_entity_count(&state->sys_name));

    component_type_id = array_get(&state->required_components, 1);
    entity_add_component(handle, *component_type_id);
    test_assert_equal_int("system has entity, 2/3 required components", 0, system_entity_count(&state->sys_name));

    component_type_id = array_get(&state->required_components, 2);
    entity_add_component(handle, *component_type_id);
    test_assert_equal_int("system has entity, 3/3 required components", 1, system_entity_count(&state->sys_name));
}

void _test__system_entity_destroyed(void* userstate)
{
    struct SystemTestState* state = userstate;

    EntityHandle handle = entity_create();

    int component_type_id = *(int*)array_get(&state->required_components, 0);
    entity_add_component(handle, component_type_id);

    component_type_id = *(int*)array_get(&state->required_components, 1);
    entity_add_component(handle, component_type_id);

    component_type_id = *(int*)array_get(&state->required_components, 2);
    entity_add_component(handle, component_type_id);

    test_assert_equal_int("system has entity", 1, system_entity_count(&state->sys_name));

    entity_destroy(handle);

    test_assert_equal_int("system has entity", 0, system_entity_count(&state->sys_name));
}

void _test__system_update(void* userstate)
{
    struct SystemTestState* state = userstate;

    EntityHandle handle = entity_create();

    int component_type_id = *(int*)array_get(&state->required_components, 0);
    struct ECSTestComponentA* comp_a = entity_add_component(handle, component_type_id);
    component_type_id = *(int*)array_get(&state->required_components, 1);
    struct ECSTestComponentB* comp_b = entity_add_component(handle, component_type_id);
    component_type_id = *(int*)array_get(&state->required_components, 2);
    struct ECSTestComponentC* comp_c = entity_add_component(handle, component_type_id);

    comp_a->x = 2;
    comp_a->y = 7;
    comp_a->z = 10;

    systems_update();

    test_component_A_values(comp_a, 3, 8, 11);
}

void test_ecs_systems(void)
{
    struct SystemTestState state;
    testing_add_group("system");
    testing_add_test("system register",   &_setup, &_teardown, &_test__system_register, &state, sizeof(state));
    testing_add_test("system uniqueness", &_setup, &_teardown, &_test__system_uniqueness, &state, sizeof(state));
    testing_add_test("system entity required components added", &_setup__system, &_teardown__system, &_test__system_entity_required_components_added, &state, sizeof(state));
    testing_add_test("system entity destroyed", &_setup__system, &_teardown__system, &_test__system_entity_destroyed, &state, sizeof(state));
    testing_add_test("system update", &_setup__system, &_teardown__system, &_test__system_update, &state, sizeof(state));
}
