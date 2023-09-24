#include "scieppend/test/core/ecs.h"
//
//#include "scieppend/core/ecs.h"
//#include "scieppend/test/test.h"
//
//struct TestComponent
//{
//    int x;
//    int y;
//    int z;
//};
//
//static void _system_update_func(struct _Entity* sys)
//{
//}
//
//static void _setup([[maybe_unused]] void* userstate)
//{
//    ecs_init();
//}
//
//static void _teardown([[maybe_unused]] void* userstate)
//{
//    ecs_uninit();
//}
//
//static void _system_setup(void* userstate)
//{
//    _setup(userstate);
//
//    struct string sys_name;
//    string_init(&sys_name, "Test System");
//    system_register(&sys_name, &_system_update_func);
//    string_uninit(&sys_name);
//}
//
//static void _system_teardown([[maybe_unused]] void* userstate)
//{
//    _teardown(userstate);
//}
//
//static void _setup_entity([[maybe_unused]] void* userstate)
//{
//    _setup(userstate);
//}
//
//static void _teardown_entity([[maybe_unused]] void* userstate)
//{
//    _teardown(userstate);
//}
//
//static void _test_system_update([[maybe_unused]] void* userstate)
//{
//    struct SystemTestState* state = userstate;
//}
//
//static void _test_entity_create_destroy([[maybe_unused]] void* userstate)
//{
//    struct EntityTestState* state = userstate;
//
//    EntityHandle entity = entity_create();
//    entity_add_component(entity, state->component_type_id);
//    entity_remove_component(entity, state->component_type_id);
//    //entity_destroy(entity);
//}
//
//void test_ecs_systems(void)
//{
//    struct SystemTestState state;
//    testing_add_group("system");
//    testing_add_test("system update", &_setup, &_teardown, &_test_system_update, &state, sizeof(state));
//}
//
//void test_ecs_entity(void)
//{
//    testing_add_group("entity");
//    testing_add_test("entity creation", &_setup_entity, &_teardown_entity, &_test_entity_create_destroy, NULL, 0);
//}
//
//void test_ecs_run_all(void)
//{
//    test_ecs_systems();
//    test_ecs_entity();
//}
