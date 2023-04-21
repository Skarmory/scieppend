#include "scieppend/test/core/hash_map.h"

#include "scieppend/core/hash_map.h"
#include "scieppend/test/test.h"

#include <stdio.h>

struct TestItem
{
    int   i;
    float f;
};

static void _setup(void* userstate)
{
    struct HashMap* hash_map = userstate;
    hash_map_init(hash_map, sizeof(struct TestItem));
}

static void _teardown(void* userstate)
{
    struct HashMap* hash_map = userstate;
    hash_map_uninit(hash_map);
}

static void _setup_with_items(void* userstate, int item_count)
{
    _setup(userstate);

    struct HashMap* hash_map = userstate;

    for(int i = 0; i < item_count; ++i)
    {
        struct TestItem ti;
        ti.i = ((i + 1) * 7);
        ti.f = 7777.f - (float)i;
        hash_map_add(hash_map, &i, sizeof(i), &ti);
    }
}

static void _setup_with_items_no_resize(void* userstate)
{
    _setup_with_items(userstate, 8);
}

static void _setup_with_items_resize(void* userstate)
{
    _setup_with_items(userstate, 32);
}

static void _teardown_with_items(void* userstate)
{
    _teardown(userstate);
}

static void _test_test_item_values(struct TestItem* ti, int expect_i, int expect_f)
{
    test_assert_equal_int("i value", expect_i, ti->i);
    test_assert_equal_float("f value", expect_f, ti->f);
}

static void _test_hash_map__add__no_resize(void* userstate)
{
    struct HashMap* hash_map = userstate;

    test_assert_equal_int("count", 0, hash_map_count(hash_map));

    const int item_count = (int)(16.0f * 0.7f);

    for(int i = 0; i < item_count; ++i)
    {
        struct TestItem t;
        t.i = 7 * (i+1);
        t.f = 7777.0f - (7.0f * i);

        hash_map_add(hash_map, &i, sizeof(i), &t);
    }

    test_assert_equal_int("count", item_count, hash_map_count(hash_map));
}

static void _test_hash_map__add__resize(void* userstate)
{
    struct HashMap* hash_map = userstate;

    test_assert_equal_int("count", 0, hash_map_count(hash_map));

    const int item_count = 32;
    for(int i = 0; i < item_count; ++i)
    {
        struct TestItem t;
        t.i = 7 * (i+1);
        t.f = 7777.0f - (7.0f * i);

        hash_map_add(hash_map, &i, sizeof(i), &t);
    }

    test_assert_equal_int("count", item_count, hash_map_count(hash_map));
}

void test_hash_map_add(void)
{
    struct HashMap hash_map;
    testing_add_group("hash map add");
    testing_add_test("no resize", &_setup, &_teardown, &_test_hash_map__add__no_resize, &hash_map, sizeof(hash_map));
    testing_add_test("resize", &_setup, &_teardown, &_test_hash_map__add__resize, &hash_map, sizeof(hash_map));
}

static void _test_hash_map__get__no_resize(void* userstate)
{
    struct HashMap* hash_map = userstate;

    for(int key = 0; key < 8; ++key)
    {
        struct TestItem ti = hash_map_get(hash_map, &key, sizeof(int), struct TestItem);
        _test_test_item_values(&ti, (key + 1) * 7, 7777.0f - (float)key);
    }
}

static void _test_hash_map__get__resize(void* userstate)
{
    struct HashMap* hash_map = userstate;

    for(int key = 0; key < 32; ++key)
    {
        struct TestItem ti = hash_map_get(hash_map, &key, sizeof(int), struct TestItem);
        _test_test_item_values(&ti, (key + 1) * 7, 7777.0f - (float)key);
    }
}

void test_hash_map_get(void)
{
    struct HashMap hash_map;
    testing_add_group("hash map get");
    testing_add_test("no resize", &_setup_with_items_no_resize, &_teardown_with_items, &_test_hash_map__get__no_resize, &hash_map, sizeof(hash_map));
    testing_add_test("resize", &_setup_with_items_resize, &_teardown_with_items, &_test_hash_map__get__resize, &hash_map, sizeof(hash_map));
}

void test_hash_map_run_all(void)
{
    test_hash_map_add();
    test_hash_map_get();
}
