#include "scieppend/test/core/array.h"

#include "scieppend/core/array.h"
#include "scieppend/core/comparator.h"
#include "scieppend/test/test.h"
#include <stddef.h>

struct ArrayTestItem
{
    int i;
    float f;
};

static int _compare_array_test_item(const void* lhs, const void* rhs)
{
    const struct ArrayTestItem* _lhs = lhs;
    const struct ArrayTestItem* _rhs = rhs;

    return compare_int(&_lhs->i, &_rhs->i);
}

static void _setup_array(void* userstate)
{
    array_init(userstate, sizeof(struct ArrayTestItem), 4, NULL, NULL);
}

static void _teardown_array(void* userstate)
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
    testing_add_test("add_no_realloc", &_setup_array, &_teardown_array, &_test_array_add_no_realloc, &test_array, sizeof(struct Array));
    testing_add_test("add_with_realloc", &_setup_array, &_teardown_array, &_test_array_add_with_realloc, &test_array, sizeof(struct Array));
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

// ARRAY FIND SORTED

static void _setup_find_sorted(void* userstate)
{
    struct Array* array = userstate;
    array_init(array, sizeof(int), 10, NULL, NULL);

    for(int i = 0; i < 10; ++i)
    {
        int test_item = (i+1) * 7;
        array_add(array, &test_item);
    }
}

static void _test_array__find_sorted(void* userstate)
{
    struct Array* array = userstate;

    const int expect_indices[10] = {  1, 0,  3,  2,  5,  4,  7,  6,  9,  8 };
    const int expect_values[10] =  { 14, 7, 28, 21, 42, 35, 56, 49, 70, 63 };

    for(int i = 0; i < 10; ++i)
    {
        int found_idx = array_find_sorted(array, &expect_values[i], &compare_int);
        test_assert_equal_int("index", expect_indices[i], found_idx);
    }

    int invalid_item = 999;
    int found_idx = array_find_sorted(array, &invalid_item, &compare_int);
    test_assert_equal_int("find value not in array index", -1, found_idx);
}

static void _test_array__find_sorted__array_size_one(void* userstate)
{
    struct Array* array = userstate;
    struct ArrayTestItem test_item = { 7, 77.0f };
    array_add(array, &test_item);
    test_assert_equal_int("array count", 1, array_count(array));
    int found_idx = array_find_sorted(array, &test_item, &_compare_array_test_item);
    test_assert_equal_int("index", 0, found_idx);
}

static void _test_array__find_sorted__array_size_zero(void* userstate)
{
    struct Array* array = userstate;
    struct ArrayTestItem test_item = { 7, 77.0f };

    test_assert_equal_int("array count", 0, array_count(array));
    int found_idx = array_find_sorted(array, &test_item, &_compare_array_test_item);
    test_assert_equal_int("index", -1, found_idx);
}

void test_array_find_sorted(void)
{
    struct Array array;
    testing_add_group("find sorted");
    testing_add_test("find sorted", &_setup_find_sorted, &_teardown_array, &_test_array__find_sorted, &array, sizeof(struct Array));
    testing_add_test("find sorted, array size 0", &_setup_array, &_teardown_array, &_test_array__find_sorted__array_size_zero, &array, sizeof(struct Array));
    testing_add_test("find sorted, array size 1", &_setup_array, &_teardown_array, &_test_array__find_sorted__array_size_one, &array, sizeof(struct Array));
}

// RUN ALL

void test_array_run_all(void)
{
    test_array_add();
    test_array_find_sorted();
}
