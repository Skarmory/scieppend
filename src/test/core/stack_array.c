#include "scieppend/test/core/stack_array.h"

#include "scieppend/core/stack_array.h"
#include "scieppend/test/test.h"
#include <string.h>

struct ArrayTestItem
{
    int i;
    float f;
};

static void _test_stackarray_add([[maybe_unused]] void* userstate)
{
    StackArray(struct ArrayTestItem, 8, test_array);

    for(int i = 0; i < 8; ++i)
    {
        struct ArrayTestItem new_item;
        new_item.i = 8 - i;
        new_item.f = (float)i * 7.0f;
        stackarray_add(&test_array, new_item);
    }

    for(int i = 0; i < 8; ++i)
    {
        test_assert_equal_int("elem int value", (8 - i), stackarray_get(&test_array, i).i);
        test_assert_equal_float("elem float value", (float)i * 7.0f, stackarray_get(&test_array, i).f);
    }
}

void test_stackarray_add(void)
{
    testing_add_group("stack array add");
    testing_add_test("add", NULL, NULL, &_test_stackarray_add, NULL, 0);
}

static void _test_stackarray_remove([[maybe_unused]] void* userstate)
{
    StackArray(int, 8, test_array);

    for(int i = 0; i < 8; ++i)
    {
        stackarray_add(&test_array, 7 * (i+1));
    }

    test_assert_equal_int("stackarray count", 8, test_array.count);
    test_assert_equal_int("stackarray idx 3 value before remove", 28, stackarray_get(&test_array, 3));
    test_assert_equal_int("stackarray idx 7 value before remove", 56, stackarray_get(&test_array, 7));

    stackarray_remove(&test_array, 3);

    test_assert_equal_int("stackarray count", 7, test_array.count);
    test_assert_equal_int("stackarray idx 3 value after remove", 56, stackarray_get(&test_array, 3));
    test_assert_equal_int("stackarray idx 7 value after remove", '\0', stackarray_get(&test_array, 7));
}

void test_stackarray_remove(void)
{
    testing_add_group("stack array remove");
    testing_add_test("remove", NULL, NULL, &_test_stackarray_remove, NULL, 0);
}

static void _test_stackarray_clear([[maybe_unused]] void* userstate)
{
    StackArray(int, 8, test_array);

    for(int i = 0; i < 8; ++i)
    {
        stackarray_add(&test_array, 7 * (i+1));
        test_assert_equal_int("stackarray values", 7*(i+1), stackarray_get(&test_array, i));
    }

    test_assert_equal_int("stackarray count", 8, test_array.count);

    stackarray_clear(&test_array);

    for(int i = 0; i < 8; ++i)
    {
        test_assert_equal_int("stackarray values", '\0', stackarray_get(&test_array, i));
    }

    test_assert_equal_int("stackarray count", 0, test_array.count);
}

void test_stackarray_clear(void)
{
    testing_add_group("stack array clear");
    testing_add_test("clear", NULL, NULL, &_test_stackarray_clear, NULL, 0);
}

void test_stackarray_run_all(void)
{
    test_stackarray_add();
    test_stackarray_remove();
    test_stackarray_clear();
}
