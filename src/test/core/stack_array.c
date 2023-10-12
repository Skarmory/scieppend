#include "scieppend/test/core/stack_array.h"

#include "scieppend/core/stack_array.h"
#include "scieppend/test/test.h"
#include <string.h>

struct ArrayTestItem
{
    int i;
    float f;
};

static void _test_stack_array_add__on_stack([[maybe_unused]] void* userstate)
{
    StackArray(struct ArrayTestItem, 32, test_array);

    for(int i = 0; i < 32; ++i)
    {
        struct ArrayTestItem new_item;
        new_item.i = 32 - i;
        new_item.f = (float)i * 7.0f;
        stackarray_add(&test_array, &new_item);
    }

    for(int i = 0; i < 32; ++i)
    {
        test_assert_equal_int("elem int value", (32 - i), stackarray_get(&test_array, i)->i);
        test_assert_equal_float("elem float value", (float)i * 7.0f, stackarray_get(&test_array, i)->f);
    }
}

void test_stack_array_add(void)
{
    testing_add_group("stack array add");
    testing_add_test("add on stack", NULL, NULL, &_test_stack_array_add__on_stack, NULL, 0);
}

void test_stack_array_run_all(void)
{
    test_stack_array_add();
}
