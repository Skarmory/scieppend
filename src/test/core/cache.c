#include "scieppend/test/core/cache.h"

#include "scieppend/core/cache.h"
#include "scieppend/test/test.h"

#include <stdarg.h>
#include <stddef.h>

struct TestItem
{
    int   i;
    float f;
};

static bool _test_cache_iterator__cache_no_gaps(void)
{
    bool success = true;
    struct Cache* cache = cache_new(sizeof(struct TestItem), 32, NULL, NULL);

    for(int i = 0; i < 32; ++i)
    {
        struct TestItem item;
        item.i = 32 - i;
        item.f = (float)i * 7.0f;
        cache_add(cache, &item);
    }

    int i = 0;
    for(struct CacheIt it = cache_begin(cache); !cache_it_eq(it, cache_end(cache)); it = cache_it_next(it))
    {
        struct TestItem* item = cache_it_get(it);
        success &= test_assert_equal_int(32 - i, item->i);
        success &= test_assert_equal_float((float)i * 7.0f, item->f);
        ++i;
    }

    cache_free(cache);

    return success;
}

bool _test_cache_iterator__cache_with_gaps(void)
{
    bool success = true;
    struct Cache* cache = cache_new(sizeof(struct TestItem), 8, NULL, NULL);

    int handles[8];

    for(int i = 0; i < 8; ++i)
    {
        struct TestItem item;
        item.i = 8 - i;
        item.f = (float)i * 7.0f;
        handles[i] = cache_add(cache, &item);
    }

    for(int i = 0; i < 8; i += 2)
    {
        cache_remove(cache, handles[i]);
    }

    int i = 1;
    for(struct CacheIt it = cache_begin(cache); !cache_it_eq(it, cache_end(cache)); it = cache_it_next(it))
    {
        struct TestItem* item = cache_it_get(it);
        success &= test_assert_equal_int(8 - i, item->i);
        success &= test_assert_equal_float((float)i * 7.0f, item->f);
        i += 2;
    }

    cache_free(cache);
    return success;
}

bool _test_cache_iterator__no_valid_items__items_removed(void)
{
    bool success = true;
    struct Cache* cache = cache_new(sizeof(struct TestItem), 8, NULL, NULL);
    int handles[8];

    for(int i = 0; i < 8; ++i)
    {
        struct TestItem item;
        item.i = 8 - i;
        item.f = (float)i * 7.0f;
        handles[i] = cache_add(cache, &item);
    }

    for(int i = 0; i < 8; ++i)
    {
        cache_remove(cache, handles[i]);
    }

    success &= test_assert_equal_int(0, cache_size(cache));

    int i = 0;
    for(struct CacheIt it = cache_begin(cache); !cache_it_eq(it, cache_end(cache)); it = cache_it_next(it))
    {
        ++i;
    }

    success &= test_assert_equal_int(0, i);

    cache_free(cache);
    return success;
}

bool test_cache_iterator(void)
{
    bool success = true;
    success &= test_run_test("cache iterator, no gaps", &_test_cache_iterator__cache_no_gaps, NULL, NULL);
    success &= test_run_test("cache iterator, cache with gaps", &_test_cache_iterator__cache_with_gaps, NULL, NULL);
    success &= test_run_test("cache iterator, no valid elements, items removed", &_test_cache_iterator__no_valid_items__items_removed, NULL, NULL);
    return success;
}

void test_cache_run_all(void)
{
    test_run_test_block("cache iterator", &test_cache_iterator);
}
