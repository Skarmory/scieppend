#include "scieppend/test/core/ecs_common.h"

#include "scieppend/core/hash.h"

#include "scieppend/test/test.h"

#include <string.h>

const char* C_TEST_COMPONENT_A_NAME = "TestComponentA";
const char* C_TEST_COMPONENT_B_NAME = "TestComponentB";
const char* C_TEST_COMPONENT_C_NAME = "TestComponentC";

int G_TEST_COMPONENT_A_ID = -1;
int G_TEST_COMPONENT_B_ID = -1;
int G_TEST_COMPONENT_C_ID = -1;

void ecs_common_init(void)
{
    G_TEST_COMPONENT_A_ID = hash(C_TEST_COMPONENT_A_NAME, strlen(C_TEST_COMPONENT_A_NAME));
    G_TEST_COMPONENT_B_ID = hash(C_TEST_COMPONENT_B_NAME, strlen(C_TEST_COMPONENT_B_NAME));
    G_TEST_COMPONENT_C_ID = hash(C_TEST_COMPONENT_C_NAME, strlen(C_TEST_COMPONENT_C_NAME));
}

void ecs_common_uninit(void)
{
}

bool test_component_A_values(const struct ECSTestComponentA* component, int expect_x, int expect_y, int expect_z)
{
    bool success = false;
    success |= test_assert_equal_int("component x", expect_x, component->x);
    success |= test_assert_equal_int("component y", expect_y, component->y);
    success |= test_assert_equal_int("component z", expect_z, component->z);
    return success;
}
