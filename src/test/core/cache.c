#include "scieppend/test/core/cache.h"

#include "scieppend/core/cache.h"
#include "scieppend/test/test.h"

#include <stdarg.h>
#include <stddef.h>

#define TEST_ELEMENTS_MAX 32

struct TestItem
{
    int   i;
    float f;
};

struct CacheTestState
{
    struct Cache* cache;
    int           handles[TEST_ELEMENTS_MAX];
};

static void _setup_cache([[maybe_unused]] void* userstate)
{
    struct CacheTestState* state = userstate;
    state->cache = cache_new(sizeof(struct TestItem), 32, NULL, NULL);

    for(int i = 0; i < TEST_ELEMENTS_MAX; ++i)
    {
        struct TestItem item;
        item.i = 32 - i;
        item.f = (float)i * 7.0f;
        state->handles[i] = cache_add(state->cache, &item);
    }
}

static void _teardown_cache([[maybe_unused]] void* userstate)
{
    struct CacheTestState* state = userstate;
    cache_free(state->cache);
}

static void _test_cache_add__resize(void* userstate)
{
    struct CacheTestState* state = userstate;

    test_assert_equal_int("cache count equal capacity", cache_size(state->cache), cache_capacity(state->cache));

    struct TestItem t;
    t.i = 0;
    t.f = 32.0f * 7.0f;
    cache_add(state->cache, &t);

    test_assert_equal_int("cache new capacity", 64, cache_capacity(state->cache));

    int i = 0;
    for(struct CacheIt it = cache_begin(state->cache); !cache_it_eq(it, cache_end(state->cache)); it = cache_it_next(it))
    {
        struct TestItem* item = cache_it_get(it);
        test_assert_equal_int("elem int value", 32 - i, item->i);
        test_assert_equal_float("elem float value", (float)i * 7.0f, item->f);
        ++i;
    }
}

void test_cache_add(void)
{
    struct CacheTestState userstate;
    testing_add_group("cache add");
    testing_add_test("add with resize", &_setup_cache, &_teardown_cache, &_test_cache_add__resize, &userstate, sizeof(struct CacheTestState));
}

static void _test_cache_iterator__cache_no_gaps([[maybe_unused]] void* userstate)
{
    struct CacheTestState* state = userstate;

    int i = 0;
    for(struct CacheIt it = cache_begin(state->cache); !cache_it_eq(it, cache_end(state->cache)); it = cache_it_next(it))
    {
        struct TestItem* item = cache_it_get(it);
        test_assert_equal_int("elem int value", 32 - i, item->i);
        test_assert_equal_float("elem float value", (float)i * 7.0f, item->f);
        ++i;
    }
}

static void _test_cache_iterator__cache_with_gaps([[maybe_unused]] void* userstate)
{
    struct CacheTestState* state = userstate;

    for(int i = 0; i < TEST_ELEMENTS_MAX; i += 2)
    {
        cache_remove(state->cache, state->handles[i]);
    }

    int i = 1;
    for(struct CacheIt it = cache_begin(state->cache); !cache_it_eq(it, cache_end(state->cache)); it = cache_it_next(it))
    {
        struct TestItem* item = cache_it_get(it);
        test_assert_equal_int("elem int value", TEST_ELEMENTS_MAX - i, item->i);
        test_assert_equal_float("elem float value", (float)i * 7.0f, item->f);
        i += 2;
    }
}

static void _test_cache_iterator__no_valid_items__items_removed([[maybe_unused]] void* userstate)
{
    struct CacheTestState* state = userstate;

    for(int i = 0; i < TEST_ELEMENTS_MAX; ++i)
    {
        cache_remove(state->cache, state->handles[i]);
    }

    test_assert_equal_int("cache size", 0, cache_size(state->cache));

    int i = 0;
    for(struct CacheIt it = cache_begin(state->cache); !cache_it_eq(it, cache_end(state->cache)); it = cache_it_next(it))
    {
        ++i;
    }

    test_assert_equal_int("iterator index", 0, i);
}

void test_cache_iterator(void)
{
    struct CacheTestState userstate;

    testing_add_group("cache iterator");
    testing_add_test("cache iterator, no gaps", &_setup_cache, &_teardown_cache, &_test_cache_iterator__cache_no_gaps, &userstate, sizeof(struct CacheTestState));
    testing_add_test("cache iterator, cache with gaps", &_setup_cache, &_teardown_cache, &_test_cache_iterator__cache_with_gaps, &userstate, sizeof(struct CacheTestState));
    testing_add_test("cache iterator, no valid elements, items removed", &_setup_cache, &_teardown_cache, &_test_cache_iterator__no_valid_items__items_removed, &userstate, sizeof(struct CacheTestState));
}


void test_cache_run_all(void)
{
    test_cache_add();
    test_cache_iterator();
}
