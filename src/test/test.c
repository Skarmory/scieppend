#include "scieppend/test/test.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C_MSG_LENGTH 256

static const char* C_SUCCESS_STR = "SUCCESS";
static const char* C_FAILURE_STR = "FAILURE";
static const int   C_SUCCESS_STR_SIZE = 7;

static struct
{
    FILE*  logfile;
    int    indent;
    int    longest_output_string;

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

static void _add_test_case(bool success, const char* format, ...)
{
    struct TestCase* casenode = malloc(sizeof(struct TestCase));
    casenode->next = NULL;
    casenode->success = success;

    va_list args;
    va_start(args, format);
    vsnprintf(casenode->msg, TEST_NAME_MAX+ TEST_MSG_MAX, format, args);
    va_end(args);

    int msglen = strlen(casenode->msg);
    if(msglen > _testing.longest_output_string)
    {
        _testing.longest_output_string = msglen;
    }

    if(!_testing.current_test->case_tail)
    {
        _testing.current_test->case_tail = casenode;
        _testing.current_test->case_head = casenode;
        return;
    }

    _testing.current_test->case_tail->next = casenode;
    _testing.current_test->case_tail = casenode;
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

bool test_assert_equal_char_buffer(const char* case_name, const char* expect, const char* actual)
{
    int expect_len = strlen(expect);
    int actual_len = strlen(actual);

    bool success = expect_len == actual_len;

    if(expect_len != actual_len)
    {
        goto test_assert_equal_char_buffer_exit;
    }

    for(int i = 0; i < expect_len; ++i)
    {
        if(expect[i] != actual[i])
        {
            success = TEST_CASE_FAIL;
            goto test_assert_equal_char_buffer_exit;
        }
    }

    success = TEST_CASE_SUCCESS;

test_assert_equal_char_buffer_exit:
    case_name = case_name ? case_name : "Test char buffer";
    _add_test_case(success, "\t%s: expect \"%s\", actual \"%s\"", case_name, expect, actual);

    return success;
}

bool test_assert_equal_int(const char* case_name, const int expect, const int actual)
{
    bool success = expect == actual;
    case_name = case_name ? case_name : "Test integer equal";

    _add_test_case(success, "\t%s: expect \"%d\", actual \"%d\"", case_name, expect, actual);

    return success;
}

bool test_assert_equal_bool(const char* case_name, const bool expect, const bool actual)
{
    bool success = expect == actual;
    case_name = case_name ? case_name : "Test bool equal";

    _add_test_case(success, "\t%s: expect \"%s\", actual: \"%s\"", case_name, expect ? "true" : "false", actual ? "true" : "false");

    return success;
}

bool test_assert_equal_float(const char* case_name, const float expect, const float actual)
{
    bool success = (expect == actual);
    case_name = case_name ? case_name : "Test float equal";

    _add_test_case(success, "\t%s: expect \"%f\", actual \"%f\"", case_name, expect, actual);

    return success;
}

bool test_assert_not_null(const char* case_name, void* value)
{
    bool success = value != NULL;
    case_name = case_name ? case_name : "Test pointer not null";

    _add_test_case(success, "\t%s");

    return success;
}

bool test_assert_null(const char* case_name, void* value)
{
    bool success = value == NULL;
    case_name = case_name ? case_name : "Test pointer is null";

    _add_test_case(success, "\t%s");

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
    _testing.longest_output_string = 0;
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

        struct TestCase* msg = test->case_head;
        while(msg != NULL)
        {
            struct TestCase* msgnext = msg->next;
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
    testobj->case_head = NULL;
    testobj->case_tail = NULL;

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
    int output_buffer_length = _testing.longest_output_string + C_SUCCESS_STR_SIZE + 1;
    char* output = malloc(output_buffer_length);

    for(struct Test* test = _testing.list_head; test != NULL; test = test->next)
    {
        fprintf(stdout, "Test: %s\n", test->name);
        for(struct TestCase* tc = test->case_head; tc != NULL; tc = tc->next)
        {
            memset(output, ' ', output_buffer_length);
            memcpy(output, tc->msg, strlen(tc->msg));
            memcpy(output + _testing.longest_output_string + 1, _success_str(tc->success), C_SUCCESS_STR_SIZE);
            fprintf(stdout, "%s\n", output);
        }
    }

    free(output);
}
