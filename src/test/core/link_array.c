#include "scieppend/test/core/link_array.h"

#include "scieppend/core/link_array.h"
#include "scieppend/test/test.h"

#include <stdio.h>

static void _setup_linkarray([[maybe_unused]] void* userstate)
{
    linkarray_init((struct LinkArray*)userstate, sizeof(int), 8, NULL, NULL);
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
        linkarray_push_back((struct LinkArray*)userstate, &z);
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
    linkarray_push_back((struct LinkArray*)userstate, &elem);
}

static void _teardown_linkarray_single_element([[maybe_unused]] void* userstate)
{
    _teardown_linkarray(userstate);
}

static void _test_linkarray_state(struct LinkArray* array, int expect_count, int expect_capacity, bool expect_usedlist_empty, bool expect_freelist_empty)
{
    test_assert_equal_int("count", expect_count, array->count);
    test_assert_equal_int("capacity", expect_capacity, array->capacity);
    test_assert_equal_bool("used list empty", expect_usedlist_empty, array->usedhead == -1);
    test_assert_equal_bool("free list empty", expect_freelist_empty, array->freehead == -1);
}

static void _test_linkarray__push_back__to_capacity([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;

    for(int i = 0; i < 8; ++i)
    {
        int z = i * 7;
        linkarray_push_back(test_array, &z);
    }

    _test_linkarray_state(test_array, 8, 8, false, true);

    for(int i = 0; i < 8; ++i)
    {
        test_assert_equal_int("elem value", i * 7, linkarray_at(test_array, i, int));
    }
}

static void _test_linkarray__push_back__beyond_capacity([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;

    int capacity = test_array->capacity + 1;
    for(int i = 0; i < capacity; ++i)
    {
        int z = i * 7;
        linkarray_push_back(test_array, &z);
    }

    _test_linkarray_state(test_array, 9, 16, false, false);

    for(int i = 0; i < linkarray_count(test_array); ++i)
    {
        test_assert_equal_int("elem value", i * 7, linkarray_at(test_array, i, int));
    }
}

static void _test_linkarray__push_front__to_capacity([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;

    for(int i = 0; i < 8; ++i)
    {
        int z = i * 7;
        linkarray_push_front(test_array, &z);
    }

    _test_linkarray_state(test_array, 8, 8, false, true);

    for(int i = 0; i < 8; ++i)
    {
        test_assert_equal_int("elem value", (7 - i) * 7, linkarray_at(test_array, i, int));
    }
}

static void _test_linkarray__push_front__beyond_capacity([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;

    int capacity = test_array->capacity + 1;
    for(int i = 0; i < capacity; ++i)
    {
        int z = i * 7;
        linkarray_push_front(test_array, &z);
    }

    _test_linkarray_state(test_array, 9, 16, false, false);

    for(int i = 0; i < linkarray_count(test_array); ++i)
    {
        test_assert_equal_int("elem value", (8 - i) * 7, linkarray_at(test_array, i, int));
    }
}

static void _test_linkarray__pop_front__single_element([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;
    _test_linkarray_state(test_array, 1, 8, false, false);
    test_assert_equal_int("front before pop", 7, linkarray_front(test_array, int));
    linkarray_pop_front(test_array);
    _test_linkarray_state(test_array, 0, 8, true, false);
}

static void _test_linkarray__pop_front__many_elements([[maybe_unused]] void* userstate)
{
    struct LinkArray* test_array = userstate;

    _test_linkarray_state(test_array, 8, 8, false, true);

    for(int i = 0; i < 8; ++i)
    {
        int expect_before = i * 7;

        test_assert_equal_int("front before pop", expect_before, linkarray_front(test_array, int));
        linkarray_pop_front(test_array);
    }

    _test_linkarray_state(test_array, 0, 8, true, false);
}

void test_linkarray_push_back(void)
{
    struct LinkArray test_array;
    testing_add_group("linkarray push back");
    testing_add_test("to capacity", &_setup_linkarray, &_teardown_linkarray, &_test_linkarray__push_back__to_capacity, &test_array, sizeof(test_array));
    testing_add_test("beyond capacity", &_setup_linkarray, &_teardown_linkarray, &_test_linkarray__push_back__beyond_capacity, &test_array, sizeof(test_array));
}

void test_linkarray_push_front(void)
{
    struct LinkArray test_array;
    testing_add_group("linkarray push front");
    testing_add_test("to capacity", &_setup_linkarray, &_teardown_linkarray, &_test_linkarray__push_front__to_capacity, &test_array, sizeof(test_array));
    testing_add_test("beyond capacity", &_setup_linkarray, &_teardown_linkarray, &_test_linkarray__push_front__beyond_capacity, &test_array, sizeof(test_array));
}

void test_linkarray_pop_front(void)
{
    struct LinkArray test_array;
    testing_add_group("linkarray pop front");
    testing_add_test("pop front single element", &_setup_linkarray_single_element, &_teardown_linkarray_single_element, &_test_linkarray__pop_front__single_element, &test_array, sizeof(test_array));
    testing_add_test("pop front many elements", &_setup_linkarray_many_elements, &_teardown_linkarray_many_elements, &_test_linkarray__pop_front__many_elements, &test_array, sizeof(test_array));
}

static void _test_linkarray__clear__no_elements(void* userstate)
{
    struct LinkArray* test_array = userstate;
    test_assert_equal_int("count", 0, test_array->count);

    linkarray_clear(test_array);

    test_assert_equal_int("count", 0, test_array->count);
}

static void _test_linkarray__clear__many_elements(void* userstate)
{
    struct LinkArray* test_array = userstate;
    test_assert_nequal_int("count", 0, test_array->count);

    linkarray_clear(test_array);

    test_assert_equal_int("count", 0, test_array->count);
}

void test_linkarray_clear(void)
{
    struct LinkArray test_array;
    testing_add_group("linkarray clear");
    testing_add_test("clear, no elements", &_setup_linkarray, &_teardown_linkarray, &_test_linkarray__clear__no_elements, &test_array, sizeof(test_array));
    testing_add_test("clear, many elements", &_setup_linkarray_many_elements, &_teardown_linkarray_many_elements, &_test_linkarray__clear__many_elements, &test_array, sizeof(test_array));
}

void _test_linkarray_iterator__traversal__no_gaps(void* userstate)
{
    struct LinkArray* array = userstate;

    int i = 0;
    for(struct LinkArrayIt it = linkarray_begin(array); !linkarray_it_eq(it, linkarray_end(array)); it = linkarray_it_next(it))
    {
        int expect = i * 7;
        int actual = linkarray_it_get(it, int);
        test_assert_equal_int("elem value", expect, actual);

        ++i;
    }
}

void _test_linkarray_iterator__traversal__some_elements_removed(void* userstate)
{
    struct LinkArray* array = userstate;

    for(int i = 0; i < 4; ++i)
    {
        linkarray_pop_at(array, i+1);
    }

    int expect = 0;
    for(struct LinkArrayIt it = linkarray_begin(array); !linkarray_it_eq(it, linkarray_end(array)); it = linkarray_it_next(it))
    {
        test_assert_equal_int("elem value", expect * 7, linkarray_it_get(it, int));
        expect += 2;
    }

    _test_linkarray_state(array, 4, 8, false, false);
}

void _test_linkarray_iterator__traversal__all_elements_removed(void* userstate)
{
    struct LinkArray* array = userstate;

    linkarray_clear(array);

    int iterations = 0;
    for(struct LinkArrayIt it = linkarray_begin(array); !linkarray_it_eq(it, linkarray_end(array)); it = linkarray_it_next(it))
    {
        ++iterations;
    }

    test_assert_equal_int("iterations", 0, iterations);
}

void test_linkarray_iterator(void)
{
    struct LinkArray test_array;
    testing_add_group("linkarray iterator");
    testing_add_test("traversal, no gaps", &_setup_linkarray_many_elements, &_teardown_linkarray_many_elements, &_test_linkarray_iterator__traversal__no_gaps, &test_array, sizeof(test_array));
    testing_add_test("traversal, some elements removed", &_setup_linkarray_many_elements, &_teardown_linkarray_many_elements, &_test_linkarray_iterator__traversal__some_elements_removed, &test_array, sizeof(test_array));
    testing_add_test("traversal, all elements removed", &_setup_linkarray_many_elements, &_teardown_linkarray_many_elements, &_test_linkarray_iterator__traversal__all_elements_removed, &test_array, sizeof(test_array));
}

void test_linkarray_run_all(void)
{
    test_linkarray_push_back();
    test_linkarray_push_front();
    test_linkarray_pop_front();
    test_linkarray_clear();
    test_linkarray_iterator();
}
