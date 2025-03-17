#include "scieppend/test/core/ecs_common.h"

#include "scieppend/core/hash.h"

#include "scieppend/test/test.h"

#include <string.h>

COMPONENT_TYPE_DEF(ECSTestComponentA);
COMPONENT_TYPE_DEF(ECSTestComponentB);
COMPONENT_TYPE_DEF(ECSTestComponentC);

void ecs_common_init(void)
{
    COMPONENT_TYPE_INIT(ECSTestComponentA);
    COMPONENT_TYPE_INIT(ECSTestComponentB);
    COMPONENT_TYPE_INIT(ECSTestComponentC);
}

void ecs_common_uninit(void)
{
    COMPONENT_TYPE_UNINIT(ECSTestComponentA);
    COMPONENT_TYPE_UNINIT(ECSTestComponentB);
    COMPONENT_TYPE_UNINIT(ECSTestComponentC);
}

bool test_component_A_values(const struct ECSTestComponentA* component, int expect_x, int expect_y, int expect_z)
{
    bool success = false;
    success |= test_assert_equal_int("component x", expect_x, component->x);
    success |= test_assert_equal_int("component y", expect_y, component->y);
    success |= test_assert_equal_int("component z", expect_z, component->z);
    return success;
}
