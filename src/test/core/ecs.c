#include "scieppend/test/core/ecs.h"

#include "scieppend/core/ecs.h"
#include "scieppend/test/test.h"

struct TestComponent
{
    int x;
    int y;
    int z;
};

static int _test_system_update_var1 = 0;
static int _test_system_update_var2 = 0;
static int _test_system_update_var3 = 0;

static ComponentTypeHandle test_component_type_id;

static void _system_update_func1(struct System* sys)
{
    ++_test_system_update_var1;
}

static void _system_update_func2(struct System* sys)
{
    ++_test_system_update_var2;
}

static void _system_update_func3(struct System* sys)
{
    ++_test_system_update_var3;
}

static void _setup(void)
{
    ecs_init();
}

static void _setup_entity(void)
{
    _setup();
    test_component_type_id = component_type_register(sizeof(struct TestComponent));
}

static void _teardown(void)
{
    ecs_uninit();
}

static void _teardown_entity(void)
{
    _teardown();
}

static bool _test_system_update(void)
{
    bool success = true;

    system_register(&_system_update_func1);
    system_register(&_system_update_func2);
    system_register(&_system_update_func3);

    systems_update();

    success &= test_assert_equal_int(1, _test_system_update_var1);
    success &= test_assert_equal_int(1, _test_system_update_var2);
    success &= test_assert_equal_int(1, _test_system_update_var3);

    systems_update();

    success &= test_assert_equal_int(2, _test_system_update_var1);
    success &= test_assert_equal_int(2, _test_system_update_var2);
    success &= test_assert_equal_int(2, _test_system_update_var3);

    return success;
}

bool _test_entity_create_destroy(void)
{
    bool success = true;

    EntityHandle entity = entity_create();
    entity_add_component(entity, test_component_type_id);
    entity_remove_component(entity, test_component_type_id);
    //entity_destroy(entity);

    return success;
}

bool test_ecs_systems(void)
{
    bool success = true;
    success &= test_run_test("system update", &_test_system_update, &_setup, &_teardown);
    return success;
}

bool test_ecs_entity(void)
{
    bool success = true;
    success &= test_run_test("entity creation", &_test_entity_create_destroy, &_setup_entity, &_teardown_entity);
    return success;
}

void test_ecs_run_all(void)
{
    test_run_test_block("Systems", &test_ecs_systems);
    test_run_test_block("Entity", &test_ecs_entity);
}
