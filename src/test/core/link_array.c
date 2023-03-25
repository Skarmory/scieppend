#include "scieppend/test/core/link_array.h"

#include "scieppend/core/link_array.h"
#include "scieppend/test/test.h"

#include <stdio.h>

static void _setup_linkarray([[maybe_unused]] void* userstate)
{
    linkarray_init((struct LinkArray*)userstate, sizeof(int), 8);
}

static void _teardown_linkarray([[maybe_unused]] void* userstate)
{
    linkarray_uninit((struct LinkArray*)userstate);
}

static void _setup_linkarray_many_elements([[maybe_unused]] void* userstate)
{
    _setup_linkarray(userstate);

    for(int i = 0; i < 8; ++i)
    {
        int z = i * 7;
        linkarray_add((struct LinkArray*)userstate, &z);
    }
}

static void _teardown_linkarray_many_elements([[maybe_unused]] void* userstate)
{
    _teardown_linkarray(userstate);
}

static void _setup_linkarray_single_element([[maybe_unused]] void* userstate)
{
    _setup_linkarray(userstate);
    int elem = 7;
    linkarray_add((struct LinkArray*)userstate, &elem);
}

static void _teardown_linkarray_single_element([[maybe_unused]] void* userstate)
{
    _teardown_linkarray(userstate);
}

static void _test_linkarray_state(struct LinkArray* array, int expect_count, int expect_capacity, int expect_usedhead, int expect_freehead)
{
    test_assert_equal_int("count", expect_count, array->count);
    test_assert_equal_int("capacity", expect_capacity, array->capacity);
    test_assert_equal_int("used head index", expect_usedhead, array->usedhead);
    test_assert_equal_int("free head index", expect_freehead, array->freehead);
}

static void _test_linkarray__add__to_capacity([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;

    for(int i = 0; i < 8; ++i)
    {
        int z = i * 7;
        linkarray_add(test_array, &z);
    }

    test_assert_equal_int("count", 8, test_array->count);
    test_assert_equal_int("capacity", 8, test_array->capacity);
    test_assert_equal_int("free head index", -1, test_array->freehead);
}

static void _test_linkarray__add__beyond_capacity([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;

    test_assert_equal_int("array count", 0, test_array->count);
    test_assert_equal_int("array capacity", 8, test_array->capacity);

    int capacity = test_array->capacity + 1;
    for(int i = 0; i < capacity; ++i)
    {
        int z = i * 7;
        linkarray_add(test_array, &z);
    }

    test_assert_equal_int("array count", 9, test_array->count);
    test_assert_equal_int("array capacity", 16, test_array->capacity);
}

static void _test_linkarray__pop_front__single_element([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;
    int elem = linkarray_pop_front(test_array, int);
    test_assert_equal_int("popped elem", 7, elem);
    _test_linkarray_state(test_array, 0, 8, -1, 0);
}

static void _test_linkarray__pop_front__many_elements([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;

    for(int i = 0; i < 8; ++i)
    {
        int expect = (7 - i) * 7;
        int elem = linkarray_pop_front(test_array, int);
        test_assert_equal_int("popped elem", expect, elem);
    }

    _test_linkarray_state(test_array, 0, 8, -1, 0);
}

void test_linkarray_add(void)
{
    struct LinkArray test_array;
    testing_add_group("linkarray add");
    testing_add_test("add to capacity", &_setup_linkarray, &_teardown_linkarray, &_test_linkarray__add__to_capacity, &test_array, sizeof(test_array));
    testing_add_test("add beyond capacity", &_setup_linkarray, &_teardown_linkarray, &_test_linkarray__add__beyond_capacity, &test_array, sizeof(test_array));
}

void test_linkarray_pop_front(void)
{
    struct LinkArray test_array;
    testing_add_group("linkarray pop front");
    testing_add_test("pop front single element", &_setup_linkarray_single_element, &_teardown_linkarray_single_element, &_test_linkarray__pop_front__single_element, &test_array, sizeof(test_array));
    testing_add_test("pop front many elements", &_setup_linkarray_many_elements, &_teardown_linkarray_many_elements, &_test_linkarray__pop_front__many_elements, &test_array, sizeof(test_array));
}

void test_linkarray_run_all(void)
{
    test_linkarray_add();
    test_linkarray_pop_front();
}
