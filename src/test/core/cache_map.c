#include "scieppend/test/core/cache_map.h"
#include "scieppend/test/test.h"

#include "scieppend/core/cache_map.h"

struct CacheMapTestItem
{
    int   i;
    float f;
};

static bool _compare_test_item(void* lhs, void* rhs)
{
    struct CacheMapTestItem* _lhs = lhs;
    struct CacheMapTestItem* _rhs = rhs;
    return _lhs->i == _rhs->i && _lhs->f == _rhs->f;
}

struct CacheMapTestState
{
    struct CacheMap map;
};

static void _setup(void* userdata)
{
    struct CacheMapTestState* state = userdata;
    cache_map_init(&state->map, sizeof(struct CacheMapTestItem));
}

static void _teardown(void* userdata)
{
    struct CacheMapTestState* state = userdata;
    cache_map_uninit(&state->map);
}

static void _test_cache_map_item(const struct CacheMapTestItem* const expect_item, const struct CacheMapTestItem* const actual_item)
{
    test_assert_equal_int("item i", expect_item->i, actual_item->i);
    test_assert_equal_float("item f", expect_item->f, actual_item->f);
}

// ADD

static void _test_cache_map__add__without_resize(void* userdata)
{
    const int C_EXPECT_ITEMS_SIZE = 8;
    const struct CacheMapTestItem expect_items[] =
    {
        { 0, 7.0f }, { 1, 14.0f }, { 2, 21.0f }, { 3, 28.0f },
        { 4, 35.0f }, { 5, 42.0f }, { 6, 49.0f }, { 7, 56.0f }
    };

    struct CacheMapTestState* state = userdata;

    for(int i = 0; i < C_EXPECT_ITEMS_SIZE; ++i)
    {
        cache_map_add(&state->map, &expect_items[i].i, sizeof(int), &expect_items[i]);
    }

    test_assert_equal_int("count", C_EXPECT_ITEMS_SIZE, cache_map_count(&state->map));

    for(int i = 0; i < C_EXPECT_ITEMS_SIZE; ++i)
    {
        const struct CacheMapTestItem* expect_item = &expect_items[i];
        const struct CacheMapTestItem* actual_item = cache_map_get(&state->map, &i, sizeof(int));

        _test_cache_map_item(expect_item, actual_item);
    }
}

static void _test_cache_map__add__with_resize(void* userdata)
{
}

void test_cache_map_add(void)
{
    struct CacheMapTestState state;
    testing_add_group("CacheMap::add");
    testing_add_test("without resize", &_setup, &_teardown, &_test_cache_map__add__without_resize, &state, sizeof(state));
    //testing_add_test("with resize", &_setup, &_teardown, &_test_cache_map__add__with_resize, &state, sizeof(state));
}

static void _test_cache_map__remove__without_resize(void* userdata)
{
    const int C_TEST_ITEMS_SIZE = 8;
    const struct CacheMapTestItem test_items[] =
    {
        { 0, 7.0f }, { 1, 14.0f }, { 2, 21.0f }, { 3, 28.0f },
        { 4, 35.0f }, { 5, 42.0f }, { 6, 49.0f }, { 7, 56.0f }
    };

    const int C_EXPECT_ITEMS_SIZE = 7;
    const struct CacheMapTestItem C_EXPECT_ITEMS[] =
    {
        { 0, 7.0f }, { 1, 14.0f }, { 2, 21.0f }, { 3, 28.0f },
        { 4, 35.0f }, { 6, 49.0f }, { 7, 56.0f }
    };

    struct CacheMapTestState* state = userdata;

    for(int i = 0; i < C_TEST_ITEMS_SIZE; ++i)
    {
        cache_map_add(&state->map, &test_items[i].i, sizeof(int), &test_items[i]);
    }

    cache_map_remove(&state->map, &test_items[5].i, sizeof(int));

    struct It it = cache_map_begin(&state->map);
    struct It eit = cache_map_end(&state->map);
    for(; !it_eq(&it, &eit); cache_map_it_next(&it))
    {
        struct CacheMapTestItem* actual_item = cache_map_it_get(&it);
        test_assert_item_in_array("item in cache_map", C_EXPECT_ITEMS, sizeof(struct CacheMapTestItem), C_EXPECT_ITEMS_SIZE, actual_item, &_compare_test_item);
    }
}

void test_cache_map_remove(void)
{
    struct CacheMapTestState state;
    testing_add_group("CacheMap::remove");
    testing_add_test("without resize", &_setup, &_teardown, &_test_cache_map__remove__without_resize, &state, sizeof(state));
    //testing_add_test("with resize", &_setup, &_teardown, &_test_cache_map__remove__with_resize, &state, sizeof(state));
}

void test_cache_map_run_all(void)
{
    test_cache_map_add();
    test_cache_map_remove();
}

