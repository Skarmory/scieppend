#include "scieppend/test/test.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define C_MSG_LENGTH 256

static const char* C_SUCCESS_STR = "success";
static const char* C_FAILURE_STR = "failure";

static struct
{
    FILE*  logfile;
    int    indent;
} _testing;

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
    _print("Test integer equal\n");
    _push_indent();
    _print("Expect: %d\n", expect);
    _print("Actual: %d\n", actual);
    _push_indent();
    bool success = expect == actual;
    _pop_indent();
    _pop_indent();
    return success;
}

bool test_assert_equal_bool(const bool expect, const bool actual)
{
    _print("Test bool equal\n");
    _push_indent();
    _print("Expect: %s\n", expect ? "true" : "false");
    _print("Actual: %s\n", actual ? "true" : "false");
    _push_indent();
    bool success = expect == actual;
    _pop_indent();
    _pop_indent();
    return success;
}

bool test_assert_equal_float(const float expect, const float actual)
{
    bool success = (expect == actual);
    _push_indent();
    _print("Test float equal: expect \"%f\", actual \"%f\"\t%s\n", expect, actual, _success_str(success));
    _pop_indent();
    return success;
}

bool test_assert_not_null(void* value)
{
    _print("Test pointer not null\n");
    _push_indent();
    bool success = value != NULL;
    _pop_indent();
    return success;
}

bool test_assert_null(void* value)
{
    _print("Test pointer is null\n");
    _push_indent();
    bool success = value == NULL;
    _pop_indent();
    return success;
}

void test_run_test_block(const char* block_name, test_func func)
{
    _print("Test Block: %s\n", block_name);
    _push_indent();
    bool success = func();
    _pop_indent();
    _print(success ? "SUCCESS\n" : "FAILED\n");
}

bool test_run_test(const char* test_name, test_func test, setup_func setup, teardown_func teardown)
{
    _print("Test: %s\n", test_name);
    _push_indent();

    if(setup)
    {
        setup();
    }

    bool success = test();

    if(teardown)
    {
        teardown();
    }

    _pop_indent();
    _print(success ? "SUCCESS\n" : "FAILED\n");

    return success;
}

void test_init(void)
{
    _testing.logfile = stdout;
    _testing.indent = 0;
}

void test_uninit(void)
{
}
