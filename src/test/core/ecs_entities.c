#include "scieppend/test/core/ecs.h"

#include "scieppend/core/array.h"
#include "scieppend/core/cache.h"
#include "scieppend/core/ecs_world.h"
#include "scieppend/core/hash.h"
#include "scieppend/core/string.h"
#include "scieppend/test/core/ecs_common.h"
#include "scieppend/test/test.h"

#include <stdio.h>

struct EntityTestState
{
    struct ECSWorld* world;
    struct Array     entity_handles;
    struct Array     dummy_component_type_ids;
};

static void _setup(void* userstate)
{
    ecs_common_init();

    struct EntityTestState* state = userstate;
    state->world = ecs_world_new();

    array_init(&state->entity_handles, sizeof(EntityHandle), 8, NULL, NULL);
    array_init(&state->dummy_component_type_ids, sizeof(ComponentTypeHandle), 10, NULL, NULL);

    ecs_world_component_type_register(state->world, COMPONENT_TYPE_ID(ECSTestComponentA), sizeof(struct ECSTestComponentA));
    ecs_world_component_type_register(state->world, COMPONENT_TYPE_ID(ECSTestComponentB), sizeof(struct ECSTestComponentB));

    for(int i = 0; i < 8; ++i)
    {
        struct string dummy_component_name;
        string_init(&dummy_component_name, "");
        string_format(&dummy_component_name, "dummy_component_%d", i);

        int id = string_hash(&dummy_component_name);
        ecs_world_component_type_register(state->world, id, 1);
        array_add(&state->dummy_component_type_ids, &id);

        string_uninit(&dummy_component_name);
    }
}

static void _teardown(void* userstate)
{
    struct EntityTestState* state = userstate;

    for(int i = 0; i < array_count(&state->entity_handles); ++i)
    {
        EntityHandle entity_handle = *(EntityHandle*)array_get(&state->entity_handles, i);
        ecs_world_destroy_entity(state->world, entity_handle);
    }

    array_uninit(&state->dummy_component_type_ids);
    array_uninit(&state->entity_handles);

    ecs_world_free(state->world);

    ecs_common_uninit();
}

static bool _add_component_and_test(struct ECSWorld* world, EntityHandle entity_handle, ComponentTypeHandle component_type_handle)
{
    bool success = false;
    char case_name[32];
    snprintf(case_name, 32, "%d count", component_type_handle);

    int component_count = ecs_world_entity_components_count(world, entity_handle);
    ecs_world_entity_add_component(world, entity_handle, component_type_handle);

    const void* component = ecs_world_entity_get_component(world, entity_handle, component_type_handle, READ);
    success |= test_assert_not_null("entity has component", component);
    success |= test_assert_equal_int("entity components count", component_count + 1, ecs_world_entity_components_count(world, entity_handle));
    ecs_world_entity_unget_component(world, entity_handle, component_type_handle, READ);

    return success;
}

static bool _remove_component_and_test(struct ECSWorld* world, EntityHandle entity_handle, ComponentTypeHandle component_type_handle)
{
    bool success = false;
    char case_name[32];
    snprintf(case_name, 32, "%d count", component_type_handle);

    int component_count = ecs_world_entity_components_count(world, entity_handle);
    ecs_world_entity_remove_component(world, entity_handle, component_type_handle);

    const void* component = ecs_world_entity_get_component(world, entity_handle, component_type_handle, READ);
    if(component != NULL)
    {
        int x = 5;
    }
    success |= test_assert_null("entity does not have component", component);
    success |= test_assert_equal_int("entity components count", component_count - 1, ecs_world_entity_components_count(world, entity_handle));

    return success;
}

static bool _get_component_and_test(struct ECSWorld* world, EntityHandle entity_handle, ComponentTypeHandle component_type_handle)
{
    char case_name[64];

    snprintf(case_name, 64, "entity has component %d", component_type_handle);
    const void* component = ecs_world_entity_get_component(world, entity_handle, component_type_handle, READ);
    bool success = test_assert_not_null(case_name, component);
    ecs_world_entity_unget_component(world, entity_handle, component_type_handle, READ);

    return success;
}

static void _test__entity_destroy_invalid_handle(void* userstate)
{
    struct EntityTestState* state = userstate;

    test_assert_equal_int("before call entity count", 0, ecs_world_entities_count(state->world));
    ecs_world_destroy_entity(state->world, C_NULL_COMPONENT_TYPE);
    test_assert_equal_int("after call entity count", 0, ecs_world_entities_count(state->world));
}

static void _test__entity_create_destroy_no_components(void* userstate)
{
    struct EntityTestState* state = userstate;

    test_assert_equal_int("entity count", 0, ecs_world_entities_count(state->world));

    EntityHandle entity_handle = ecs_world_create_entity(state->world);
    test_assert_nequal_int("entity handle", C_NULL_CACHE_HANDLE, entity_handle);
    test_assert_equal_int("entity count", 1, ecs_world_entities_count(state->world));

    ecs_world_destroy_entity(state->world, entity_handle);
    test_assert_equal_int("entity count", 0, ecs_world_entities_count(state->world));
}

static void _test__entity_add_remove_component(void* userstate)
{
    struct EntityTestState* state = userstate;

    EntityHandle entity_handle = ecs_world_create_entity(state->world);

    {
        _add_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
        struct ECSTestComponentA* component = ecs_world_entity_get_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), WRITE);
        component->x = 7;
        component->y = 1234;
        component->z = 9876;
        ecs_world_entity_unget_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), WRITE);
    }

    {
        const struct ECSTestComponentA* component = ecs_world_entity_get_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), READ);
        test_component_A_values(component, 7, 1234, 9876);
        ecs_world_entity_unget_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), READ);
        _remove_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
    }

    {
        _add_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
        struct ECSTestComponentA* component = ecs_world_entity_get_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), WRITE);
        component->x = 1357;
        component->y = 2468;
        component->z = 3579;
        ecs_world_entity_unget_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), WRITE);
    }

    {
        const struct ECSTestComponentA* component = ecs_world_entity_get_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), READ);
        test_component_A_values(component, 1357, 2468, 3579);
        ecs_world_entity_unget_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), READ);
        _remove_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
    }

    ecs_world_destroy_entity(state->world, entity_handle);
}

static void _test__entity_add_remove_component_with_resize(void* userstate)
{
    struct EntityTestState* state = userstate;

    EntityHandle entity_handle = ecs_world_create_entity(state->world);

    _add_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
    _add_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentB));
    //for(int i = 0; i < array_count(&state->dummy_component_type_ids); ++i)
    //{
    //    _add_component_and_test(state->world, entity_handle, *(ComponentTypeHandle*)array_get(&state->dummy_component_type_ids, i));
    //}

    _get_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
    _get_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentB));
    //for(int i = 0; i < array_count(&state->dummy_component_type_ids); ++i)
    //{
    //    _get_component_and_test(state->world, entity_handle, *(ComponentTypeHandle*)array_get(&state->dummy_component_type_ids, i));
    //}

    _remove_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
    _remove_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentB));
    //for(int i = array_count(&state->dummy_component_type_ids) - 1; i > -1; --i)
    //{
    //    _remove_component_and_test(state->world, entity_handle, *(ComponentTypeHandle*)array_get(&state->dummy_component_type_ids, i));
    //}
}

static void _test__entity_component_uniqueness(void* userstate)
{
    struct EntityTestState* state = userstate;

    EntityHandle entity_handle = ecs_world_create_entity(state->world);

    _add_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));

    int entity_components = ecs_world_entity_components_count(state->world, entity_handle);
    int components = ecs_world_components_count(state->world, COMPONENT_TYPE_ID(ECSTestComponentA));

    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
    test_assert_equal_int("entity component count", entity_components, ecs_world_entity_components_count(state->world, entity_handle));
    test_assert_equal_int("ecs components count", components, ecs_world_components_count(state->world, COMPONENT_TYPE_ID(ECSTestComponentA)));

    _remove_component_and_test(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));

    ecs_world_destroy_entity(state->world, entity_handle);
}

void test_ecs_entities(void)
{
    struct EntityTestState entity_test_state;
    testing_add_group("entity");
    testing_add_test("entity destroy with invalid handle", &_setup, &_teardown, &_test__entity_destroy_invalid_handle, &entity_test_state, sizeof(entity_test_state));
    testing_add_test("entity create and destroy", &_setup, &_teardown, &_test__entity_create_destroy_no_components, &entity_test_state, sizeof(entity_test_state));
    testing_add_test("entity add and remove component", &_setup, &_teardown, &_test__entity_add_remove_component, &entity_test_state, sizeof(entity_test_state));
    testing_add_test("entity add and remove components with resize", &_setup, &_teardown, &_test__entity_add_remove_component_with_resize, &entity_test_state, sizeof(entity_test_state));
    testing_add_test("entity component uniqueness", &_setup, &_teardown, &_test__entity_component_uniqueness, &entity_test_state, sizeof(entity_test_state));
}
