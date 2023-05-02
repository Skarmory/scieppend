#include "scieppend/test/core/link_array.h"

#include "scieppend/core/link_array.h"
#include "scieppend/test/test.h"

#include <stdio.h>
#include <stdlib.h>

static const int C_DEFAULT_CAPACITY = 8;
static const int C_CAPACITY_AFTER_1_RESIZE = C_DEFAULT_CAPACITY << 1;

struct LinkArrayTestItem
{
    int i;
    float f;
};

struct LinkArrayTestState
{
    struct LinkArray array;
};

static struct LinkArrayTestItem _make_test_item(int index)
{
    struct LinkArrayTestItem item;
    item.i = (index + 1) * 7;
    item.f = (index + 1) * 77.0f;
    return item;
}

static struct LinkArrayTestItem* _malloc_test_item(int index)
{
    struct LinkArrayTestItem* item = malloc(sizeof(struct LinkArrayTestItem));
    *item = _make_test_item(index);
    return item;
}

// SETUP/TEARDOWN

static void _setup_linkarray_non_ptr_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;
    linkarray_init(&state->array, sizeof(struct LinkArrayTestItem), C_DEFAULT_CAPACITY, NULL, NULL);
}

static void _teardown_linkarray_non_ptr_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;
    linkarray_uninit(&state->array);
}

static void _setup_linkarray_ptr_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;
    linkarray_init(&state->array, sizeof(struct LinkArrayTestItem*), C_DEFAULT_CAPACITY, NULL, NULL);
}

static void _teardown_linkarray_ptr_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;
    linkarray_uninit(&state->array);
}

static void _setup_linkarray_with_elements_no_resize(void* userstate)
{
    _setup_linkarray_non_ptr_elements(userstate);

    struct LinkArrayTestState* state = userstate;

    for(int i = 0; i < C_DEFAULT_CAPACITY; ++i)
    {
        struct LinkArrayTestItem item = _make_test_item(i);
        linkarray_push_back(&state->array, &item);
    }
}

static void _teardown_linkarray_with_elements(void* userstate)
{
    _teardown_linkarray_non_ptr_elements(userstate);
}

static void _setup_linkarray_single_element(void* userstate)
{
    _setup_linkarray_non_ptr_elements(userstate);

    struct LinkArrayTestItem item = _make_test_item(0);
    linkarray_push_back((struct LinkArray*)userstate, &item);
}

static void _teardown_linkarray_single_element(void* userstate)
{
    _teardown_linkarray_non_ptr_elements(userstate);
}

static void _setup_linkarray_with_ptr_elements(void* userdata)
{
    struct LinkArrayTestState* state = userdata;

    _setup_linkarray_ptr_elements(state);

    for(int i = 0; i < C_DEFAULT_CAPACITY; ++i)
    {
        struct LinkArrayTestItem* item = _malloc_test_item(i);
        linkarray_push_back(&state->array, &item);
    }
}

static void _teardown_linkarray_with_ptr_elements(void* userdata)
{
    struct LinkArrayTestState* state= userdata;

    _teardown_linkarray_ptr_elements(&state->array);
}

static void _test_element_value(const struct LinkArrayTestItem* item, const struct LinkArrayTestItem* expect)
{
    test_assert_equal_int("elem value i", expect->i, item->i);
    test_assert_equal_float("elem value f", expect->f, item->f);
}

/* Check all elements in expect_values are in array, in the same order.
 */
static void _test_linkarray_non_ptr_values(struct LinkArray* array, const struct LinkArrayTestItem* expect_values, int expect_values_count)
{
    test_assert_equal_int("expect values count", expect_values_count, linkarray_count(array));

    for(int i = 0; i < expect_values_count; ++i)
    {
        struct LinkArrayTestItem item = linkarray_at(array, i, struct LinkArrayTestItem);
        _test_element_value(&item, &expect_values[i]);
    }
}

/* Check all elements in expect_values are in array, in the same order.
 */
static void _test_linkarray_ptr_values(struct LinkArray* array, const struct LinkArrayTestItem* expect_values, int expect_values_count)
{
    test_assert_equal_int("expect values count", expect_values_count, linkarray_count(array));

    for(int i = 0; i < expect_values_count; ++i)
    {
        struct LinkArrayTestItem* item = linkarray_at(array, i, struct LinkArrayTestItem*);
        _test_element_value(item, &expect_values[i]);
    }
}

static void _test_linkarray_state(struct LinkArray* array, int expect_count, int expect_capacity, bool expect_usedlist_empty, bool expect_freelist_empty)
{
    test_assert_equal_int("count", expect_count, array->count);
    test_assert_equal_int("capacity", expect_capacity, array->capacity);
    test_assert_equal_bool("used list empty", expect_usedlist_empty, array->usedhead == -1);
    test_assert_equal_bool("free list empty", expect_freelist_empty, array->freehead == -1);
}

// AT

static void _test_linkarray__at__non_ptr_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 7, 77.0f }, { 14, 154.0f }, { 21, 231.0f }, { 28, 308.0f }, { 35, 385.0f }, { 42, 462.0f }, { 49, 539.0f }, { 56, 616.0f }
    };

    _test_linkarray_non_ptr_values(&state->array, expect_values, C_DEFAULT_CAPACITY);
}

static void _test_linkarray__at__ptr_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 7, 77.0f }, { 14, 154.0f }, { 21, 231.0f }, { 28, 308.0f }, { 35, 385.0f }, { 42, 462.0f }, { 49, 539.0f }, { 56, 616.0f }
    };

    _test_linkarray_ptr_values(&state->array, expect_values, C_DEFAULT_CAPACITY);
}

void test_linkarray_at(void)
{
    struct LinkArrayTestState state;
    testing_add_group("linkarray at");
    testing_add_test("at, nonptr items", &_setup_linkarray_with_elements_no_resize, &_teardown_linkarray_with_elements, &_test_linkarray__at__non_ptr_elements, &state, sizeof(state));
    testing_add_test("at, ptr items", &_setup_linkarray_with_ptr_elements, &_teardown_linkarray_with_ptr_elements, &_test_linkarray__at__ptr_elements, &state, sizeof(state));
}

// PUSH BACK

static void _test_linkarray__push_back__to_capacity(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 7, 77.0f }, { 14, 154.0f }, { 21, 231.0f }, { 28, 308.0f }, { 35, 385.0f }, { 42, 462.0f }, { 49, 539.0f }, { 56, 616.0f }
    };

    for(int i = 0; i < C_DEFAULT_CAPACITY; ++i)
    {
        struct LinkArrayTestItem item = _make_test_item(i);
        linkarray_push_back(&state->array, &item);
    }

    _test_linkarray_state(&state->array, 8, C_DEFAULT_CAPACITY, false, true);
    _test_linkarray_non_ptr_values(&state->array, expect_values, C_DEFAULT_CAPACITY);
}

static void _test_linkarray__push_back__beyond_capacity(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 7, 77.0f }, { 14, 154.0f }, { 21, 231.0f }, { 28, 308.0f }, { 35, 385.0f }, { 42, 462.0f }, { 49, 539.0f }, { 56, 616.0f }, { 63, 693.0f }
    };

    int count = state->array.capacity + 1;
    for(int i = 0; i < count; ++i)
    {
        struct LinkArrayTestItem item = _make_test_item(i);
        linkarray_push_back(&state->array, &item);
    }

    _test_linkarray_state(&state->array, 9, C_CAPACITY_AFTER_1_RESIZE, false, false);
    _test_linkarray_non_ptr_values(&state->array, expect_values, count);
}

void test_linkarray_push_back(void)
{
    struct LinkArrayTestState state;
    testing_add_group("linkarray push back");
    testing_add_test("to capacity", &_setup_linkarray_non_ptr_elements, &_teardown_linkarray_non_ptr_elements, &_test_linkarray__push_back__to_capacity, &state, sizeof(state));
    testing_add_test("beyond capacity", &_setup_linkarray_non_ptr_elements, &_teardown_linkarray_non_ptr_elements, &_test_linkarray__push_back__beyond_capacity, &state, sizeof(state));
}

// PUSH FRONT

static void _test_linkarray__push_front__to_capacity(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 56, 616.0f }, { 49, 539.0f }, { 42, 462.0f }, { 35, 385.0f }, { 28, 308.0f }, { 21, 231.0f }, { 14, 154.0f }, { 7, 77.0f }
    };

    for(int i = 0; i < C_DEFAULT_CAPACITY; ++i)
    {
        struct LinkArrayTestItem item = _make_test_item(i);
        linkarray_push_front(&state->array, &item);
    }

    _test_linkarray_state(&state->array, C_DEFAULT_CAPACITY, C_DEFAULT_CAPACITY, false, true);
    _test_linkarray_non_ptr_values(&state->array, expect_values, C_DEFAULT_CAPACITY);
}

static void _test_linkarray__push_front__beyond_capacity(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 63, 693.0f }, { 56, 616.0f }, { 49, 539.0f }, { 42, 462.0f }, { 35, 385.0f }, { 28, 308.0f }, { 21, 231.0f }, { 14, 154.0f }, { 7, 77.0f }
    };

    int count = state->array.capacity + 1;
    for(int i = 0; i < count; ++i)
    {
        struct LinkArrayTestItem item = _make_test_item(i);
        linkarray_push_front(&state->array, &item);
    }

    _test_linkarray_state(&state->array, C_DEFAULT_CAPACITY + 1, C_CAPACITY_AFTER_1_RESIZE, false, false);
    _test_linkarray_non_ptr_values(&state->array, expect_values, count);
}

void test_linkarray_push_front(void)
{
    struct LinkArrayTestState state;
    testing_add_group("linkarray push front");
    testing_add_test("to capacity", &_setup_linkarray_non_ptr_elements, &_teardown_linkarray_non_ptr_elements, &_test_linkarray__push_front__to_capacity, &state, sizeof(state));
    testing_add_test("beyond capacity", &_setup_linkarray_non_ptr_elements, &_teardown_linkarray_non_ptr_elements, &_test_linkarray__push_front__beyond_capacity, &state, sizeof(state));
}

// POP BACK

static void _test_linkarray__pop_back__single_element(void* userstate)
{
    struct LinkArrayTestState* state = userstate;
    _test_linkarray_state(&state->array, 1, C_DEFAULT_CAPACITY, false, false);
    test_assert_equal_int("back before pop", 7, linkarray_back(&state->array, int));
    linkarray_pop_back(&state->array);
    _test_linkarray_state(&state->array, 0, C_DEFAULT_CAPACITY, true, false);
}

static void _test_linkarray__pop_back__many_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 7, 77.0f }, { 14, 154.0f }, { 21, 231.0f }, { 28, 308.0f }, { 35, 385.0f }, { 42, 462.0f }, { 49, 539.0f }, { 56, 616.0f }, { 63, 693.0f }
    };

    _test_linkarray_state(&state->array, C_DEFAULT_CAPACITY, C_DEFAULT_CAPACITY, false, true);

    for(int i = 0; i < C_DEFAULT_CAPACITY; ++i)
    {
        struct LinkArrayTestItem item = linkarray_back(&state->array, struct LinkArrayTestItem);
        test_assert_equal_int("back i before pop", expect_values[C_DEFAULT_CAPACITY - i - 1].i, item.i);
        test_assert_equal_float("back f before pop", expect_values[C_DEFAULT_CAPACITY - i - 1].f, item.f);
        linkarray_pop_back(&state->array);
    }

    _test_linkarray_state(&state->array, 0, 8, true, false);
}

void test_linkarray_pop_back(void)
{
    struct LinkArrayTestState state;
    testing_add_group("linkarray pop back");
    testing_add_test("pop back single element", &_setup_linkarray_single_element, &_teardown_linkarray_single_element, &_test_linkarray__pop_back__single_element, &state, sizeof(state));
    testing_add_test("pop back many elements", &_setup_linkarray_with_elements_no_resize, &_teardown_linkarray_with_elements, &_test_linkarray__pop_back__many_elements, &state, sizeof(state));
}

// POP FRONT

static void _test_linkarray__pop_front__single_element(void* userstate)
{
    struct LinkArrayTestState* state = userstate;
    _test_linkarray_state(&state->array, 1, 8, false, false);
    test_assert_equal_int("front before pop", 7, linkarray_front(&state->array, int));
    linkarray_pop_front(&state->array);
    _test_linkarray_state(&state->array, 0, 8, true, false);
}

static void _test_linkarray__pop_front__many_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 7, 77.0f }, { 14, 154.0f }, { 21, 231.0f }, { 28, 308.0f }, { 35, 385.0f }, { 42, 462.0f }, { 49, 539.0f }, { 56, 616.0f }, { 63, 693.0f }
    };

    _test_linkarray_state(&state->array, 8, 8, false, true);

    for(int i = 0; i < C_DEFAULT_CAPACITY; ++i)
    {
        const struct LinkArrayTestItem* expect = &expect_values[i];
        struct LinkArrayTestItem actual = linkarray_front(&state->array, struct LinkArrayTestItem);
        test_assert_equal_int("front before pop, integer", expect->i, actual.i);
        test_assert_equal_int("front before pop, float", expect->f, actual.f);
        linkarray_pop_front(&state->array);
    }

    _test_linkarray_state(&state->array, 0, C_DEFAULT_CAPACITY, true, false);
}

void test_linkarray_pop_front(void)
{
    struct LinkArrayTestState state;
    testing_add_group("linkarray pop front");
    testing_add_test("pop front single element", &_setup_linkarray_single_element, &_teardown_linkarray_single_element, &_test_linkarray__pop_front__single_element, &state, sizeof(state));
    testing_add_test("pop front many elements", &_setup_linkarray_with_elements_no_resize, &_teardown_linkarray_with_elements, &_test_linkarray__pop_front__many_elements, &state, sizeof(state));
}

// CLEAR

static void _test_linkarray__clear__no_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;
    test_assert_equal_int("count", 0, state->array.count);
    linkarray_clear(&state->array);
    test_assert_equal_int("count", 0, state->array.count);
}

static void _test_linkarray__clear__many_elements(void* userstate)
{
    struct LinkArrayTestState* state = userstate;
    test_assert_equal_int("count", C_DEFAULT_CAPACITY, state->array.count);
    linkarray_clear(&state->array);
    test_assert_equal_int("count", 0, state->array.count);
}

void test_linkarray_clear(void)
{
    struct LinkArrayTestState state;
    testing_add_group("linkarray clear");
    testing_add_test("clear, no elements", &_setup_linkarray_non_ptr_elements, &_teardown_linkarray_non_ptr_elements, &_test_linkarray__clear__no_elements, &state, sizeof(state));
    testing_add_test("clear, many elements", &_setup_linkarray_with_elements_no_resize, &_teardown_linkarray_with_elements, &_test_linkarray__clear__many_elements, &state, sizeof(state));
}

// ITERATOR

void _test_linkarray_iterator__traversal__no_gaps(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 7, 77.0f }, { 14, 154.0f }, { 21, 231.0f }, { 28, 308.0f }, { 35, 385.0f }, { 42, 462.0f }, { 49, 539.0f }, { 56, 616.0f }
    };

    int i = 0;
    for(struct LinkArrayIt it = linkarray_begin(&state->array); !linkarray_it_eq(it, linkarray_end(&state->array)); it = linkarray_it_next(it))
    {
        struct LinkArrayTestItem item = linkarray_it_get(it, struct LinkArrayTestItem);
        _test_element_value(&item, &expect_values[i]);
        ++i;
    }
}

void _test_linkarray_iterator__traversal__some_elements_removed(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    static const struct LinkArrayTestItem expect_values[] =
    {
        { 7, 77.0f }, { 21, 231.0f }, { 35, 385.0f }, { 49, 539.0f }
    };

    // Remove every other item
    for(int i = 0; i < 4; ++i)
    {
        linkarray_pop_at(&state->array, i+1);
    }

    int i = 0;
    for(struct LinkArrayIt it = linkarray_begin(&state->array); !linkarray_it_eq(it, linkarray_end(&state->array)); it = linkarray_it_next(it))
    {
        struct LinkArrayTestItem actual = linkarray_it_get(it, struct LinkArrayTestItem);
        _test_element_value(&actual, &expect_values[i]);
        ++i;
    }

    _test_linkarray_state(&state->array, 4, C_DEFAULT_CAPACITY, false, false);
}

void _test_linkarray_iterator__traversal__all_elements_removed(void* userstate)
{
    struct LinkArrayTestState* state = userstate;

    linkarray_clear(&state->array);

    int iterations = 0;
    for(struct LinkArrayIt it = linkarray_begin(&state->array); !linkarray_it_eq(it, linkarray_end(&state->array)); it = linkarray_it_next(it))
    {
        ++iterations;
    }

    test_assert_equal_int("iterations", 0, iterations);
}

void test_linkarray_iterator(void)
{
    struct LinkArrayTestState test_state;
    testing_add_group("linkarray iterator");
    testing_add_test("traversal, no gaps", &_setup_linkarray_with_elements_no_resize, &_teardown_linkarray_with_elements, &_test_linkarray_iterator__traversal__no_gaps, &test_state, sizeof(test_state));
    testing_add_test("traversal, some elements removed", &_setup_linkarray_with_elements_no_resize, &_teardown_linkarray_with_elements, &_test_linkarray_iterator__traversal__some_elements_removed, &test_state, sizeof(test_state));
    testing_add_test("traversal, all elements removed", &_setup_linkarray_with_elements_no_resize, &_teardown_linkarray_with_elements, &_test_linkarray_iterator__traversal__all_elements_removed, &test_state, sizeof(test_state));
}

void test_linkarray_run_all(void)
{
    test_linkarray_at();
    test_linkarray_push_back();
    test_linkarray_push_front();
    test_linkarray_pop_back();
    test_linkarray_pop_front();
    test_linkarray_clear();
    test_linkarray_iterator();
}
