#ifndef SCIEPPEND_TEST_TEST_H
#define SCIEPPEND_TEST_TEST_H

#include <stdbool.h>

#define TEST_CASE_FAIL false
#define TEST_CASE_SUCCESS true
#define TEST_NAME_MAX 64
#define TEST_MSG_MAX 256

typedef bool(*test_func)(void);
typedef void(*setup_func)(void);
typedef void(*teardown_func)(void);

struct TestMessage
{
    struct TestMessage* next;
    char                msg[TEST_MSG_MAX];
};

struct Test
{
    struct Test*        next;
    char                name[TEST_NAME_MAX];
    setup_fn            setup;
    teardown_fn         teardown;
    test_fn             test;
    void*               userstate;
    bool                success;
    struct TestMessage* msg_head;
    struct TestMessage* msg_tail;
};

bool test_assert_equal_char_buffer(const char* expect, const char* actual);
bool test_assert_equal_int(const int expect, const int actual);
bool test_assert_equal_bool(const bool expect, const bool actual);
bool test_assert_equal_float(const float expect, const float actual);
bool test_assert_not_null(void* value);
bool test_assert_null(void* value);

void test_run_test_block(const char* block_name, test_func func);
bool test_run_test(const char* test_name, test_func test, setup_func setup, teardown_func teardown);

void test_init(void);
void test_uninit(void);

#endif
