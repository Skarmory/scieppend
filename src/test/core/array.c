#include "scieppend/test/core/array.h"

#include "scieppend/core/array.h"
#include "scieppend/test/test.h"
#include <stddef.h>

struct ArrayTestItem
{
    int i;
    float f;
};

static struct Array* s_test_array;

static void _setup_array_add(void)
{
    s_test_array = array_new(sizeof(struct ArrayTestItem), 4, NULL);
}

static void _teardown_array_add(void)
{
    array_free(s_test_array);
}

// ARRAY ADD

static bool _test_array_add_no_realloc(void)
{
    bool success = true;

    struct ArrayTestItem test_item;
    test_item.i = 7;
    test_item.f = 33.0f;

    success &= test_assert_equal_int(0, array_count(s_test_array));
    success &= test_assert_equal_int(4, array_capacity(s_test_array));

    array_add(s_test_array, &test_item);

    success &= test_assert_equal_int(1, array_count(s_test_array));
    success &= test_assert_equal_int(4, array_capacity(s_test_array));

    struct ArrayTestItem* retrieved = array_get(s_test_array, 0);
    success &= test_assert_equal_int(test_item.i, retrieved->i);
    success &= test_assert_equal_float(test_item.f, retrieved->f);

    return success;
}

static bool _test_array_add_with_realloc(void)
{
    bool success = true;

    success &= test_assert_equal_int(0, array_count(s_test_array));
    success &= test_assert_equal_int(4, array_capacity(s_test_array));

    struct ArrayTestItem test_item;
    for(int i = 0; i < 6; ++i)
    {
        test_item.i = i * 7;
        test_item.f = (float)i * 33.0f;
        array_add(s_test_array, &test_item);
    }

    success &= test_assert_equal_int(6, array_count(s_test_array));
    success &= test_assert_equal_int(8, array_capacity(s_test_array));

    for(int i = 0; i < 6; ++i)
    {
        struct ArrayTestItem* retrieved = array_get(s_test_array, i);
        success &= test_assert_equal_int(i * 7, retrieved->i);
        success &= test_assert_equal_float((float)i * 33.0f, retrieved->f);
    }

    return success;
}

bool test_array_add(void)
{
    bool success = true;
    success &= test_run_test("add_no_realloc", &_test_array_add_no_realloc, &_setup_array_add, &_teardown_array_add);
    success &= test_run_test("add_with_realloc", &_test_array_add_with_realloc, &_setup_array_add, &_teardown_array_add);
    return success;
}

// ARRAY_REMOVE

void _setup_array_remove(void)
{
}

void _teardown_array_remove(void)
{
}

bool test_array_remove(void)
{
    bool success = true;
    //success &= test_run_test("remove_from_start", &_test_array_remove_from_start, &
    return success;
}

// RUN ALL

void test_array_run_all(void)
{
    test_run_test_block("array_add", &test_array_add);
}
