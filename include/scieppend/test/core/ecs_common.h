#ifndef SCIEPPEND_TEST_CORE_ECS_COMMON_H
#define SCIEPPEND_TEST_CORE_ECS_COMMON_H

extern const char* C_TEST_COMPONENT_A_NAME;
extern const char* C_TEST_COMPONENT_B_NAME;
extern const char* C_TEST_COMPONENT_C_NAME;
extern int G_TEST_COMPONENT_A_ID;
extern int G_TEST_COMPONENT_B_ID;
extern int G_TEST_COMPONENT_C_ID;

struct ECSTestComponentA
{
    int x;
    int y;
    int z;
};

struct ECSTestComponentB
{
    float a;
    float b;
};

struct ECSTestComponentC
{
    char c[64];
};

void ecs_common_init(void);
void ecs_common_uninit(void);

bool test_component_A_values(const struct ECSTestComponentA* component, int expect_x, int expect_y, int expect_z);

#endif
