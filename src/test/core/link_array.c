#include "scieppend/test/core/link_array.h"

#include "scieppend/core/link_array.h"
#include "scieppend/test/test.h"

#include <stdio.h>

static struct LinkArray s_test_array;

static void _setup_linkarray([[maybe_unused]] void* userstate)
{
    linkarray_init(&s_test_array, sizeof(int), 8);
}

static void _teardown_linkarray([[maybe_unused]] void* userstate)
{
    linkarray_uninit(&s_test_array);
}

static void _setup_linkarray_many_elements([[maybe_unused]] void* userstate)
{
    _setup_linkarray(userstate);

    for(int i = 0; i < 8; ++i)
    {
        int z = i * 7;
        linkarray_add(&s_test_array, &z);
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
    linkarray_add(&s_test_array, &elem);
}

static void _teardown_linkarray_single_element([[maybe_unused]] void* userstate)
{
    _teardown_linkarray(userstate);
}

static bool _test_linkarray_state(int expect_count, int expect_capacity, int expect_usedhead, int expect_freehead)
{
    bool success = true;
    success &= test_assert_equal_int(expect_count, s_test_array.count);
    success &= test_assert_equal_int(expect_capacity, s_test_array.capacity);
    success &= test_assert_equal_int(expect_usedhead, s_test_array.usedhead);
    success &= test_assert_equal_int(expect_freehead, s_test_array.freehead);
    return success;
}

static bool _test_linkarray__add__to_capacity([[maybe_unused]] void* userstate)
{
    bool success = true;

    for(int i = 0; i < 8; ++i)
    {
        int z = i * 7;
        linkarray_add(&s_test_array, &z);
    }

    success &= test_assert_equal_int(8, s_test_array.count);
    success &= test_assert_equal_int(8, s_test_array.capacity);
    success &= test_assert_equal_int(-1, s_test_array.freehead);

    return success;
}

static bool _test_linkarray__add__beyond_capacity([[maybe_unused]] void* userstate)
{
    bool success = true;

    success &= test_assert_equal_int(0, s_test_array.count);
    success &= test_assert_equal_int(8, s_test_array.capacity);

    int capacity = s_test_array.capacity + 1;
    for(int i = 0; i < capacity; ++i)
    {
        int z = i * 7;
        linkarray_add(&s_test_array, &z);
    }

    success &= test_assert_equal_int(9, s_test_array.count);
    success &= test_assert_equal_int(16, s_test_array.capacity);

    return success;
}

static bool _test_linkarray__pop_front__single_element([[maybe_unused]] void* userstate)
{
    bool success = true;

    int elem = linkarray_pop_front(&s_test_array, int);
    success &= test_assert_equal_int(7, elem);
    success &= _test_linkarray_state(0, 8, -1, 0);

    return success;
}

static bool _test_linkarray__pop_front__many_elements([[maybe_unused]] void* userstate)
{
    bool success = true;

    for(int i = 0; i < 8; ++i)
    {
        int expect = (7 - i) * 7;
        int elem = linkarray_pop_front(&s_test_array, int);
        success &= test_assert_equal_int(expect, elem);
    }

    success &= _test_linkarray_state(0, 8, -1, 0);

    return success;
}

bool test_linkarray_add([[maybe_unused]] void* userstate)
{
    bool success = true;
    success &= test_run_test("add to capacity", &_test_linkarray__add__to_capacity, &_setup_linkarray, &_teardown_linkarray);
    success &= test_run_test("add beyond capacity", &_test_linkarray__add__beyond_capacity, &_setup_linkarray, &_teardown_linkarray);
    return success;
}

bool test_linkarray_pop_front([[maybe_unused]] void* userstate)
{
    bool success = true;
    success &= test_run_test("pop front single element", &_test_linkarray__pop_front__single_element, &_setup_linkarray_single_element, &_teardown_linkarray_single_element);
    success &= test_run_test("pop front many elements", &_test_linkarray__pop_front__many_elements, &_setup_linkarray_many_elements, &_teardown_linkarray_many_elements);
    return success;
}

void test_linkarray_run_all(void)
{
    test_run_test_block("linkarray add", &test_linkarray_add);
    test_run_test_block("linkarray pop front", &test_linkarray_pop_front);
}
