#ifndef SCIEPPEND_TEST_TEST_H
#define SCIEPPEND_TEST_TEST_H

#include <stdbool.h>

#define TEST_CASE_FAIL false
#define TEST_CASE_SUCCESS true
#define TEST_NAME_MAX 64
#define TEST_MSG_MAX 256

typedef void(*test_fn)(void* userstate);
typedef void(*setup_fn)(void* userstate);
typedef void(*teardown_fn)(void* userstate);

struct TestCase
{
    struct TestCase* next;
    char             msg[TEST_NAME_MAX + TEST_MSG_MAX];
    bool             success;
};

struct Test
{
    struct Test*     next;
    char             name[TEST_NAME_MAX];
    setup_fn         setup;
    teardown_fn      teardown;
    test_fn          test;
    void*            userstate;
    bool             success;
    struct TestCase* case_head;
    struct TestCase* case_tail;
};

struct TestGroup
{
    struct TestGroup* next;
    char name[TEST_NAME_MAX];
    bool success;

    struct Test* test_head;
    struct Test* test_tail;
};

bool test_assert_equal_char_buffer(const char* case_name, const char* expect, const char* actual);
bool test_assert_equal_int(const char* case_name, const int expect, const int actual);
bool test_assert_equal_bool(const char* case_name, const bool expect, const bool actual);
bool test_assert_equal_float(const char* case_name, const float expect, const float actual);
bool test_assert_not_null(const char* case_name, void* value);
bool test_assert_null(const char* case_name, void* value);

void test_init(void);
void test_uninit(void);

void testing_add_group(char name[]);
void testing_add_test(char name[], setup_fn setup, teardown_fn teardown, test_fn test, void* userstate, int userstate_size);
void testing_run_tests(void);
void testing_report(void);

#endif
