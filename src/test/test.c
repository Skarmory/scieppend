#include "scieppend/test/test.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C_MSG_LENGTH 256

static const char* C_SUCCESS_STR = "SUCCESS";
static const char* C_FAILURE_STR = "FAILURE";

static struct
{
    FILE*  logfile;
    int    indent;

    struct Test* list_head;
    struct Test* list_tail;
    struct Test* current_test;
} _testing;

static void _list_add_test(struct Test* test)
{
    if(!_testing.list_tail)
    {
        _testing.list_tail = test;
        _testing.list_head = test;
        return;
    }

    _testing.list_tail->next = test;
    _testing.list_tail = test;
}

static void _add_msg(char msg[TEST_MSG_MAX])
{
    struct TestMessage* msgnode = malloc(sizeof(struct TestMessage));
    snprintf(msgnode->msg, TEST_MSG_MAX, "%s", msg);
    msgnode->next = NULL;

    if(!_testing.current_test->msg_tail)
    {
        _testing.current_test->msg_tail = msgnode;
        _testing.current_test->msg_head = msgnode;
        return;
    }

    _testing.current_test->msg_tail->next = msgnode;
    _testing.current_test->msg_tail = msgnode;
}

static void _push_indent(void)
{
    _testing.indent++;
}

static void _pop_indent(void)
{
    if(_testing.indent > 0)
    {
        _testing.indent--;
    }
}

static void _print(char* format, ...)
{
    va_list args;

    for(int i = 0; i < _testing.indent; ++i)
    {
        fprintf(_testing.logfile, "\t");
    }

    va_start(args, format);
    vfprintf(_testing.logfile, format, args);
    va_end(args);
    fflush(_testing.logfile);
}

static const char* _success_str(bool success)
{
    return success ? C_SUCCESS_STR : C_FAILURE_STR;
}

bool test_assert_equal_char_buffer(const char* expect, const char* actual)
{
    _print("Test char buffers\n");

    _push_indent();
    _print("Expect: %s\n", expect);
    _print("Actual: %s\n", actual);

    _push_indent();

    int expect_len = strlen(expect);
    int actual_len = strlen(actual);

    if(expect_len != actual_len)
    {
        _print("Expected length %d, actual length %d\n", expect_len, actual_len);
        goto test_assert_equal_char_buffer_fail;
    }

    for(int i = 0; i < expect_len; ++i)
    {
        if(expect[i] != actual[i])
        {
            _print("Buffers differ at index %d\n", i);
            goto test_assert_equal_char_buffer_fail;
        }
    }

    _pop_indent();
    _pop_indent();
    return TEST_CASE_SUCCESS;

test_assert_equal_char_buffer_fail:
    _pop_indent();
    _pop_indent();
    return TEST_CASE_FAIL;
}

bool test_assert_equal_int(const int expect, const int actual)
{
    bool success = expect == actual;
    _print("Test integer equal: expect \"%d\", actual \"%d\"\t%s\n", expect, actual, _success_str(success));
    return success;
}

bool test_assert_equal_bool(const bool expect, const bool actual)
{
    bool success = expect == actual;
    _print("Test bool equal: expect \"%s\", actual: \"%s\"\t%s\n", expect ? "true" : "false", actual ? "true" : "false", _success_str(success));
    return success;
}

bool test_assert_equal_float(const float expect, const float actual)
{
    bool success = (expect == actual);
    _print("Test float equal: expect \"%f\", actual \"%f\"\t%s\n", expect, actual, _success_str(success));
    return success;
}

bool test_assert_not_null(void* value)
{
    bool success = value != NULL;
    _print("Test pointer not null\t%s\n", _success_str(success));
    return success;
}

bool test_assert_null(void* value)
{
    _print("Test pointer is null\n");
    bool success = value == NULL;
    return success;
}

void test_run_test_block(const char* block_name, test_fn func)
{
    _print("Test Block: %s\n", block_name);
    _push_indent();
    int* blah;
    bool success = func(blah);
    _pop_indent();
    _print(success ? "SUCCESS\n" : "FAILED\n");
}

bool test_run_test(const char* test_name, test_fn test, setup_fn setup, teardown_fn teardown)
{
    _print("Test: %s\n", test_name);
    _push_indent();

    int* blah;
    if(setup)
    {
        setup(blah);
    }

    bool success = test(blah);

    if(teardown)
    {
        teardown(blah);
    }

    _pop_indent();
    _print(success ? "SUCCESS\n" : "FAILED\n");

    return success;
}

void test_init(void)
{
    _testing.logfile = stdout;
    _testing.indent = 0;
    _testing.list_head = NULL;
    _testing.list_tail = NULL;
    _testing.current_test = NULL;
}

void test_uninit(void)
{
    struct Test* test = _testing.list_head;
    while(test != NULL)
    {
        struct Test* testnext = test->next;

        struct TestMessage* msg = test->msg_head;
        while(msg != NULL)
        {
            struct TestMessage* msgnext = msg->next;
            free(msg);
            msg = msgnext;
        }

        free(test->userstate);
        free(test);
        test = testnext;
    }
}

void testing_add_test(char name[], setup_fn setup, teardown_fn teardown, test_fn test, void* userstate, int userstate_size)
{
    struct Test* testobj = malloc(sizeof(struct Test));
    snprintf(testobj->name, TEST_NAME_MAX, "%s", name);
    testobj->setup = setup;
    testobj->teardown = teardown;
    testobj->test = test;
    testobj->userstate = malloc(userstate_size);
    if(userstate != NULL)
    {
        memcpy(testobj->userstate, userstate, userstate_size);
    }
    else
    {
        testobj->userstate = NULL;
    }
    testobj->msg_head = NULL;
    testobj->msg_tail = NULL;

    _list_add_test(testobj);
}

void testing_run_tests(void)
{
    for(struct Test* test = _testing.list_head; test != NULL; test = test->next)
    {
        _testing.current_test = test;
        _testing.current_test->setup(_testing.current_test->userstate);
        _testing.current_test->success = _testing.current_test->test(_testing.current_test->userstate);
        _testing.current_test->teardown(_testing.current_test->userstate);
    }
}

void testing_report(void)
{
    for(struct Test* test = _testing.list_head; test != NULL; test = test->next)
    for(struct TestMessage* msg = test->msg_head; msg != NULL; msg = msg->next)
    {
        fprintf(stdout, "%s\n", msg->msg);
    }
}
