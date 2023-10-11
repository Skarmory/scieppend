#include "scieppend/test/core/ecs.h"

#include "scieppend/core/cache.h"
#include "scieppend/core/ecs.h"
#include "scieppend/core/string.h"
#include "scieppend/test/core/ecs_common.h"
#include "scieppend/test/test.h"

struct EntityTestState
{
    struct Array  entity_handles;
    struct Array  dummy_component_type_names;
    struct string component_type_A_name;
    struct string component_type_B_name;
};

static void _setup([[maybe_unused]] void* userstate)
{
    ecs_init();
}

static void _teardown([[maybe_unused]] void* userstate)
{
    ecs_uninit();
}

static void _setup__entity([[maybe_unused]] void* userstate)
{
    struct EntityTestState* state = userstate;
    array_init(&state->entity_handles, sizeof(EntityHandle), 8, NULL, NULL);
    EntityHandle handle = entity_create();
    array_add(&state->entity_handles, &handle);
}

static void _teardown__entity(void* userstate)
{
    struct EntityTestState* state = userstate;
    for(int i = 0; i < array_count(&state->entity_handles); ++i)
    {
        entity_destroy(*(EntityHandle*)array_get(&state->entity_handles, i));
    }

    array_uninit(&state->entity_handles);
}

static void _setup__component_types(void* userstate)
{
    struct EntityTestState* state = userstate;

    array_init(&state->dummy_component_type_names, sizeof(struct string), 10, NULL, NULL);
    string_init(&state->component_type_A_name, C_TEST_COMPONENT_A_NAME);
    string_init(&state->component_type_B_name, C_TEST_COMPONENT_B_NAME);
    component_type_register(&state->component_type_A_name, sizeof(struct ECSTestComponentA));
    component_type_register(&state->component_type_B_name, sizeof(struct ECSTestComponentB));

    for(int i = 0; i < 8; ++i)
    {
        struct string dummy_component_name;
        string_init(&dummy_component_name, "");
        string_format(&dummy_component_name, "dummy_component_%d", i);

        component_type_register(&dummy_component_name, 1);
        array_add(&state->dummy_component_type_names, &dummy_component_name);
    }
}

static void _teardown__component_types(void* userstate)
{
    struct EntityTestState* state = userstate;

    for(int i = array_count(&state->dummy_component_type_names) - 1; i > -1 ; --i)
    {
        struct string* name = array_get(&state->dummy_component_type_names, i);
        string_uninit(name);
    }

    string_uninit(&state->component_type_A_name);
    string_uninit(&state->component_type_B_name);
    array_uninit(&state->dummy_component_type_names);
}

static void _setup__add_remove_component(void* userstate)
{
    _setup(userstate);
    _setup__component_types(userstate);
    _setup__entity(userstate);
}

static void _teardown__add_remove_component(void* userstate)
{
    _teardown__entity(userstate);
    _teardown__component_types(userstate);
    _teardown(userstate);
}

static bool _add_component_and_test(EntityHandle handle, const struct string* component_name)
{
    bool success = false;
    char case_name[32];
    snprintf(case_name, 32, "%s count", component_name->buffer);

    int component_count = entity_component_count(handle);

    entity_add_component_by_name(handle, component_name);
    void* component = entity_get_component_by_name(handle, component_name);
    success |= test_assert_not_null("entity has component", component);
    success |= test_assert_equal_int(case_name, 1, ecs_components_count(component_name));
    success |= test_assert_equal_int("entity components count", component_count + 1, entity_component_count(handle));

    return success;
}

static bool _remove_component_and_test(EntityHandle handle, const struct string* component_name)
{
    bool success = false;
    char case_name[32];
    snprintf(case_name, 32, "%s count", component_name->buffer);

    int component_count = entity_component_count(handle);

    entity_remove_component_by_name(handle, component_name);
    void* component = entity_get_component_by_name(handle, component_name);
    success |= test_assert_null("entity does not have component", component);
    success |= test_assert_equal_int(case_name, 0, ecs_components_count(component_name));
    success |= test_assert_equal_int("entity components count", component_count - 1, entity_component_count(handle));

    return success;
}

static bool _get_component_and_test(EntityHandle handle, const struct string* component_name)
{
    char case_name[64];
    snprintf(case_name, 64, "entity has component %s", component_name->buffer);

    void* component = entity_get_component_by_name(handle, component_name);
    return test_assert_not_null(case_name, component);
}

static void _test__entity_destroy_invalid_handle(void* userstate)
{
    (void)userstate;

    test_assert_equal_int("before call entity count", 0, ecs_entities_count());
    entity_destroy(-1);
    test_assert_equal_int("after call entity count", 0, ecs_entities_count());
}

static void _test__entity_create_destroy_no_components(void* userstate)
{
    (void)userstate;

    test_assert_equal_int("entity count", 0, ecs_entities_count());

    EntityHandle handle = entity_create();
    test_assert_nequal_int("entity handle", C_NULL_CACHE_HANDLE, handle);
    test_assert_equal_int("entity count", 1, ecs_entities_count());

    entity_destroy(handle);
    test_assert_equal_int("entity count", 0, ecs_entities_count());
}

static void _test__entity_add_remove_component(void* userstate)
{
    struct EntityTestState* state = userstate;

    EntityHandle handle = *(EntityHandle*)array_get(&state->entity_handles, 0);

    _add_component_and_test(handle, &state->component_type_A_name);

    struct ECSTestComponentA* component = entity_get_component_by_name(handle, &state->component_type_A_name);
    component->x = 7;
    component->y = 1234;
    component->z = 9876;

    component = NULL;
    component = entity_get_component_by_name(handle, &state->component_type_A_name);
    test_component_A_values(component, 7, 1234, 9876);
    _remove_component_and_test(handle, &state->component_type_A_name);

    component = NULL;

    _add_component_and_test(handle, &state->component_type_A_name);
    component = entity_get_component_by_name(handle, &state->component_type_A_name);
    component->x = 1357;
    component->y = 2468;
    component->z = 3579;

    component = NULL;
    component = entity_get_component_by_name(handle, &state->component_type_A_name);
    test_component_A_values(component, 1357, 2468, 3579);
    _remove_component_and_test(handle, &state->component_type_A_name);
}

static void _test__entity_add_remove_component_with_resize(void* userstate)
{
    struct EntityTestState* state = userstate;

    EntityHandle handle = *(EntityHandle*)array_get(&state->entity_handles, 0);

    _add_component_and_test(handle, &state->component_type_A_name);
    _add_component_and_test(handle, &state->component_type_B_name);
    for(int i = 0; i < array_count(&state->dummy_component_type_names); ++i)
    {
        _add_component_and_test(handle, array_get(&state->dummy_component_type_names, i));
    }

    _get_component_and_test(handle, &state->component_type_A_name);
    _get_component_and_test(handle, &state->component_type_B_name);
    for(int i = 0; i < array_count(&state->dummy_component_type_names); ++i)
    {
        _get_component_and_test(handle, array_get(&state->dummy_component_type_names, i));
    }

    _remove_component_and_test(handle, &state->component_type_A_name);
    _remove_component_and_test(handle, &state->component_type_B_name);
    for(int i = array_count(&state->dummy_component_type_names) - 1; i > -1; --i)
    {
        _remove_component_and_test(handle, array_get(&state->dummy_component_type_names, i));
    }
}

static void _test__entity_component_uniqueness(void* userstate)
{
    struct EntityTestState* state = userstate;

    EntityHandle handle = *(EntityHandle*)array_get(&state->entity_handles, 0);

    _add_component_and_test(handle, &state->component_type_A_name);

    int entity_components = entity_component_count(handle);
    int components = ecs_components_count(&state->component_type_A_name);

    entity_add_component_by_name(handle, &state->component_type_A_name);
    test_assert_equal_int("entity component count", entity_components, entity_component_count(handle));
    test_assert_equal_int("ecs components count", components, ecs_components_count(&state->component_type_A_name));

    _remove_component_and_test(handle, &state->component_type_A_name);
}

void test_ecs_entities(void)
{
    struct EntityTestState entity_test_state;
    testing_add_group("entity");
    testing_add_test("entity destroy with invalid handle", &_setup, &_teardown, &_test__entity_destroy_invalid_handle, NULL, 0);
    testing_add_test("entity create and destroy", &_setup, &_teardown, &_test__entity_create_destroy_no_components, NULL, 0);
    testing_add_test("entity add and remove component", &_setup__add_remove_component, &_teardown__add_remove_component, &_test__entity_add_remove_component, &entity_test_state, sizeof(entity_test_state));
    testing_add_test("entity add and remove components with resize", &_setup__add_remove_component, &_teardown__add_remove_component, &_test__entity_add_remove_component_with_resize, &entity_test_state, sizeof(entity_test_state));
    testing_add_test("entity component uniqueness", &_setup__add_remove_component, &_teardown__add_remove_component, &_test__entity_component_uniqueness, &entity_test_state, sizeof(entity_test_state));
}
