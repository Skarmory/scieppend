#include "scieppend/test/core/ecs.h"

#include "scieppend/core/ecs_world.h"
#include "scieppend/core/event.h"
#include "scieppend/core/string.h"
#include "scieppend/core/system.h"
#include "scieppend/test/core/ecs_common.h"
#include "scieppend/test/test.h"

struct SystemTestState
{
    struct ECSWorld* world;
};

// INTERNAL FUNCS

static void _system_update(struct ECSWorld* world, EntityHandle entity_handle)
{
    struct ECSTestComponentA* comp_a = ecs_world_entity_get_component(world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), WRITE);
    struct ECSTestComponentB* comp_b = ecs_world_entity_get_component(world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentB), WRITE);
    struct ECSTestComponentC* comp_c = ecs_world_entity_get_component(world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentC), WRITE);

    comp_a->x++;
    comp_a->y++;
    comp_a->z++;

    ecs_world_entity_unget_component(world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentC), WRITE);
    ecs_world_entity_unget_component(world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentB), WRITE);
    ecs_world_entity_unget_component(world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), WRITE);
}

static void _setup(void* userstate)
{
    ecs_common_init();
    eventing_init();

    struct SystemTestState* state = userstate;

    state->world = ecs_world_new();

    ecs_world_component_type_register(state->world, COMPONENT_TYPE_ID(ECSTestComponentA), sizeof(struct ECSTestComponentA));
    ecs_world_component_type_register(state->world, COMPONENT_TYPE_ID(ECSTestComponentB), sizeof(struct ECSTestComponentB));
    ecs_world_component_type_register(state->world, COMPONENT_TYPE_ID(ECSTestComponentC), sizeof(struct ECSTestComponentC));

    struct Array required_components;
    array_init(&required_components, sizeof(ComponentTypeHandle), 3, NULL, NULL);
    array_add(&required_components, &COMPONENT_TYPE_ID(ECSTestComponentA));
    array_add(&required_components, &COMPONENT_TYPE_ID(ECSTestComponentB));
    array_add(&required_components, &COMPONENT_TYPE_ID(ECSTestComponentC));

    struct string system_name;
    string_init(&system_name, "TestSystemName");

    ecs_world_system_register(state->world, &system_name, &required_components, &_system_update);

    string_uninit(&system_name);
    array_uninit(&required_components);
}

static void _teardown(void* userstate)
{
    struct SystemTestState* state = userstate;
    ecs_world_free(state->world);
    eventing_uninit();
}

// TESTS

void _test__system_uniqueness(void* userstate)
{
    struct SystemTestState* state = userstate;

    test_assert_equal_int("systems registered count", 1, ecs_world_systems_count(state->world));

    struct string system_name;
    string_init(&system_name, "TestSystemName");
    ecs_world_system_register(state->world, &system_name, NULL, NULL);
    string_uninit(&system_name);

    test_assert_equal_int("systems registered count", 1, ecs_world_systems_count(state->world));
}

void _test__system_entity_required_components_added(void* userstate)
{
    struct SystemTestState* state = userstate;

    struct string system_name;
    string_init(&system_name, "TestSystemName");
    struct System* system = ecs_world_get_system(state->world, &system_name);
    string_uninit(&system_name);

    EntityHandle entity_handle = ecs_world_create_entity(state->world);

    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
    test_assert_equal_int("system has entity, 1/3 required components", 0, system_entities_count(system));

    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentB));
    test_assert_equal_int("system has entity, 2/3 required components", 0, system_entities_count(system));

    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentC));
    test_assert_equal_int("system has entity, 3/3 required components", 1, system_entities_count(system));

    ecs_world_destroy_entity(state->world, entity_handle);
}

void _test__system_entity_destroyed(void* userstate)
{
    struct SystemTestState* state = userstate;

    struct string system_name;
    string_init(&system_name, "TestSystemName");

    struct System* system = ecs_world_get_system(state->world, &system_name);
    string_uninit(&system_name);

    EntityHandle entity_handle = ecs_world_create_entity(state->world);

    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));
    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentB));
    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentC));

    test_assert_equal_int("system has entity", 1, system_entities_count(system));

    ecs_world_destroy_entity(state->world, entity_handle);

    test_assert_equal_int("system has entity", 0, system_entities_count(system));
}

void _test__system_update(void* userstate)
{
    struct SystemTestState* state = userstate;

    EntityHandle entity_handle = ecs_world_create_entity(state->world);

    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA));

    struct ECSTestComponentA* comp_a = ecs_world_entity_get_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), WRITE);
    if (comp_a != NULL)
    {
        comp_a->x = 2;
        comp_a->y = 7;
        comp_a->z = 10;
        ecs_world_entity_unget_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentA), WRITE);
    }

    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentB));
    ecs_world_entity_add_component(state->world, entity_handle, COMPONENT_TYPE_ID(ECSTestComponentC));

    ecs_world_update_systems(state->world);

    test_component_A_values(comp_a, 3, 8, 11);

    ecs_world_destroy_entity(state->world, entity_handle);
}

void test_ecs_systems(void)
{
    struct SystemTestState state;
    testing_add_group("system");
    testing_add_test("system uniqueness", &_setup, &_teardown, &_test__system_uniqueness, &state, sizeof(state));
    testing_add_test("system entity required components added", &_setup, &_teardown, &_test__system_entity_required_components_added, &state, sizeof(state));
    testing_add_test("system entity destroyed", &_setup, &_teardown, &_test__system_entity_destroyed, &state, sizeof(state));
    testing_add_test("system update", &_setup, &_teardown, &_test__system_update, &state, sizeof(state));
}
