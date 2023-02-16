#include "scieppend/test/core/stack_array.h"

#include "scieppend/core/stack_array.h"
#include "scieppend/test/test.h"
#include <string.h>

struct ArrayTestItem
{
    int i;
    float f;
};

static bool _test_stack_array_add__on_stack(void)
{
    bool success = true;

    StackArray(struct ArrayTestItem, 32) test_array;
    stackarray_init(&test_array);

    for(int i = 0; i < 32; ++i)
    {
        struct ArrayTestItem new_item;
        new_item.i = 32 - i;
        new_item.f = (float)i * 7.0f;
        stackarray_add(&test_array, &new_item);
    }

    for(int i = 0; i < 32; ++i)
    {
        success &= test_assert_equal_int((32 - i), stackarray_get(&test_array, i)->i);
        success &= test_assert_equal_float((float)i * 7.0f, stackarray_get(&test_array, i)->f);
    }

    return success;
}

static bool _test_stack_array_add__on_heap(void)
{
    bool success = true;

    StackArray(struct ArrayTestItem, 32)* test_array = stackarray_new(struct ArrayTestItem, 32);
    stackarray_init(test_array);

    for(int i = 0; i < 32; ++i)
    {
        struct ArrayTestItem new_item;
        new_item.i = 32 - i;
        new_item.f = (float)i * 7.0f;
        stackarray_add(test_array, &new_item);
    }

    for(int i = 0; i < 32; ++i)
    {
        success &= test_assert_equal_int((32 - i), stackarray_get(test_array, i)->i);
        success &= test_assert_equal_float((float)i * 7.0f, stackarray_get(test_array, i)->f);
    }

    free(test_array);

    return success;
}

bool test_stack_array_add(void)
{
    bool success = true;
    success &= test_run_test("add on stack", &_test_stack_array_add__on_stack, NULL, NULL);
    success &= test_run_test("add on heap", &_test_stack_array_add__on_heap, NULL, NULL);
    return success;
}

void test_stack_array_run_all(void)
{
    test_run_test_block("stack array add", &test_stack_array_add);
}
