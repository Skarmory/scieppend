#include "scieppend/test/core/array.h"

#include "scieppend/core/array.h"
#include "scieppend/test/test.h"
#include <stddef.h>

struct ArrayTestItem
{
    int i;
    float f;
};

static void _setup_array_add([[maybe_unused]] void* userstate)
{
    array_init(userstate, sizeof(struct ArrayTestItem), 4, NULL, NULL);
}

static void _teardown_array_add([[maybe_unused]] void* userstate)
{
    array_uninit((struct Array*)userstate);
}

// ARRAY COMMON

static void _test_array_count(struct Array* array, int expect)
{
    test_assert_equal_int("array count", expect, array->count);
}

static void _test_array_capacity(struct Array* array, int expect)
{
    test_assert_equal_int("array capacity", expect, array->capacity);
}

// ARRAY ADD

static void _test_array_add_no_realloc([[maybe_unused]] void* userstate)
{
    struct Array* test_array = userstate;

    _test_array_count(test_array, 0);
    _test_array_capacity(test_array, 4);

    struct ArrayTestItem test_item;
    test_item.i = 7;
    test_item.f = 33.0f;

    array_add(test_array, &test_item);

    _test_array_count(test_array, 1);
    _test_array_capacity(test_array, 4);

    struct ArrayTestItem* retrieved = array_get(test_array, 0);
    test_assert_equal_int("elem int value", test_item.i, retrieved->i);
    test_assert_equal_float("elem float value", test_item.f, retrieved->f);
}

static void _test_array_add_with_realloc([[maybe_unused]] void* userstate)
{
    struct Array* test_array = userstate;

    _test_array_count(test_array, 0);
    _test_array_capacity(test_array, 4);

    struct ArrayTestItem test_item;
    for(int i = 0; i < 6; ++i)
    {
        test_item.i = i * 7;
        test_item.f = (float)i * 33.0f;
        array_add(test_array, &test_item);
    }

    _test_array_count(test_array, 6);
    _test_array_capacity(test_array, 8);

    for(int i = 0; i < 6; ++i)
    {
        struct ArrayTestItem* retrieved = array_get(test_array, i);
        test_assert_equal_int("elem int value", i * 7, retrieved->i);
        test_assert_equal_float("elem float value", (float)i * 33.0f, retrieved->f);
    }
}

void test_array_add(void)
{
    struct Array test_array;
    testing_add_group("dynamic array add");
    testing_add_test("add_no_realloc", &_setup_array_add, &_teardown_array_add, &_test_array_add_no_realloc, &test_array, sizeof(struct Array));
    testing_add_test("add_with_realloc", &_setup_array_add, &_teardown_array_add, &_test_array_add_with_realloc, &test_array, sizeof(struct Array));
}

// ARRAY_REMOVE

void _setup_array_remove(void)
{
}

void _teardown_array_remove(void)
{
}

void test_array_remove(void)
{
    //success &= test_run_test("remove_from_start", &_test_array_remove_from_start, &
}

// RUN ALL

void test_array_run_all(void)
{
    test_array_add();
}
