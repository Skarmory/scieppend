#ifndef SCIEPPEND_TEST_CORE_ECS_COMMON_H
#define SCIEPPEND_TEST_CORE_ECS_COMMON_H

#include "scieppend/core/component.h"

COMPONENT_TYPE_DECL(ECSTestComponentA)
{
    int x;
    int y;
    int z;
};

COMPONENT_TYPE_DECL(ECSTestComponentB)
{
    float a;
    float b;
};

COMPONENT_TYPE_DECL(ECSTestComponentC)
{
    char c[64];
};

void ecs_common_init(void);
void ecs_common_uninit(void);

bool test_component_A_values(const struct ECSTestComponentA* component, int expect_x, int expect_y, int expect_z);

#endif
