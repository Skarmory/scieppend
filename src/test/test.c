#include "scieppend/test/test.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C_MSG_LENGTH 256

static const char* C_SUCCESS_STR = "SUCCESS";
static const char* C_FAILURE_STR = "FAILURE";
static const char* C_SUCCESS_STR_COLOUR = "\033[32mSUCCESS\033[0m";
static const char* C_FAILURE_STR_COLOUR = "\033[31mFAILURE\033[0m";
static const int   C_SUCCESS_STR_SIZE_MAX = 20;

static struct
{
    FILE*  logfile;
    int    longest_output;
    bool   ansi_colours;

    struct TestGroup* group_head;
    struct TestGroup* group_tail;
    struct TestGroup* current_group;
    struct Test*      current_test;

    int success_count;
    int fail_count;
} _testing;

static void _testing_add_group(struct TestGroup* group)
{
    _testing.current_group = group;

    if(!_testing.group_tail)
    {
        _testing.group_tail = group;
        _testing.group_head = group;
        return;
    }

    _testing.group_tail->next = group;
    _testing.group_tail = group;
}

static void _testing_add_test(struct Test* test)
{
    if(!_testing.current_group->test_tail)
    {
        _testing.current_group->test_tail = test;
        _testing.current_group->test_head = test;
        return;
    }

    _testing.current_group->test_tail->next = test;
    _testing.current_group->test_tail = test;
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
    if(msglen > _testing.longest_output)
    {
        _testing.longest_output = msglen;
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

static void _default_setup([[maybe_unused]] void* userstate)
{
}

static void _default_teardown([[maybe_unused]] void* userstate)
{
}

static const char* _success_str(bool success)
{
    if(_testing.ansi_colours)
    {
        return success ? C_SUCCESS_STR_COLOUR : C_FAILURE_STR_COLOUR;
    }
    else
    {
        return success ? C_SUCCESS_STR : C_FAILURE_STR;
    }
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

bool test_assert_nequal_int(const char* case_name, const int expect, const int actual)
{
    bool success = (expect != actual);
    case_name = case_name ? case_name : "Test integer not equal";

    _add_test_case(success, "\t%s: expect not \"%d\", actual \"%d\"", case_name, expect, actual);

    return success;
}

bool test_assert_item_in_array(const char* case_name, const void* array, const int elem_bytes, const int array_count, const void* item, compare_fn comp)
{
    bool success = false;
    for(int i = 0; i < array_count; ++i)
    {
        success |= comp(array + (elem_bytes * i), item);

        if(success)
        {
            break;
        }
    }

    _add_test_case(success, "\t%s", case_name);

    return success;
}


void test_init(bool ansi_colours)
{
    _testing.logfile = stdout;
    _testing.longest_output = 0;
    _testing.ansi_colours = ansi_colours;
    _testing.group_head = NULL;
    _testing.group_tail = NULL;
    _testing.current_test = NULL;
    _testing.success_count = 0;
    _testing.fail_count = 0;
}

void test_uninit(void)
{
    struct TestGroup* group = _testing.group_head;
    while(group != NULL)
    {
        struct TestGroup* groupnext = group->next;
        struct Test* test = group->test_head;

        while(test != NULL)
        {
            struct Test* testnext = test->next;
            struct TestCase* tc = test->case_head;

            while(tc != NULL)
            {
                struct TestCase* tcnext = tc->next;
                free(tc);
                tc = tcnext;
            }

            free(test->userstate);
            free(test);
            test = testnext;
        }

        free(group);
        group = groupnext;
    }

}

void testing_add_group(char name[])
{
    struct TestGroup* group = malloc(sizeof(struct TestGroup));
    snprintf(group->name, TEST_NAME_MAX, "%s", name);
    group->success = false;
    group->next = NULL;
    group->test_head = NULL;
    group->test_tail = NULL;

    _testing_add_group(group);
}

void testing_add_test(char name[], setup_fn setup, teardown_fn teardown, test_fn test, void* userstate, int userstate_size)
{
    struct Test* testobj = malloc(sizeof(struct Test));
    snprintf(testobj->name, TEST_NAME_MAX, "%s", name);
    testobj->setup = setup ? setup : _default_setup;
    testobj->teardown = teardown ? teardown : _default_teardown;
    testobj->test = test;
    testobj->success = false;

    if(userstate != NULL)
    {
        testobj->userstate = malloc(userstate_size);
        memcpy(testobj->userstate, userstate, userstate_size);
    }
    else
    {
        testobj->userstate = NULL;
    }

    testobj->next = NULL;
    testobj->case_head = NULL;
    testobj->case_tail = NULL;

    _testing_add_test(testobj);
}

void testing_run_tests(void)
{
    if(!_testing.group_head)
    {
        fprintf(stdout, "Warning: No test groups have been added\n");
        return;
    }

    for(struct TestGroup* group = _testing.group_head; group != NULL; group = group->next)
    {
        if(group->test_head == NULL)
        {
            fprintf(stdout, "Warning: No tests have been added to test group \"%s\"\n", group->name);
            continue;
        }

        for(struct Test* test = group->test_head; test != NULL; test = test->next)
        {
            _testing.current_test = test;
            test->setup(test->userstate);
            test->test(test->userstate);
            test->teardown(test->userstate);

            if(!test->case_head)
            {
                fprintf(stdout, "Warning: No test cases found for test \"%s\", skipping evaluation\n", test->name);
                continue;
            }

            for(struct TestCase* tc = test->case_head; tc != NULL; tc = tc->next)
            {
                test->success |= tc->success;
                if(test->success)
                {
                    ++_testing.success_count;
                }
                else
                {
                    ++_testing.fail_count;
                }
            }

            group->success |= test->success;
        }
    }
}

void testing_report(void)
{
    int output_buffer_length = _testing.longest_output + C_SUCCESS_STR_SIZE_MAX + 2;
    char* output = malloc(output_buffer_length);
    memset(output, '\0', output_buffer_length);

    for(struct TestGroup* group = _testing.group_head; group != NULL; group = group->next)
    {
        fprintf(stdout, "Test group: %s\n", group->name);
        for(struct Test* test = group->test_head; test != NULL; test = test->next)
        {
            fprintf(stdout, "\tTest: %s\n", test->name);
            for(struct TestCase* tc = test->case_head; tc != NULL; tc = tc->next)
            {
                memset(output, ' ', output_buffer_length - 1);
                memcpy(output, tc->msg, strlen(tc->msg));
                memcpy(output + _testing.longest_output + 1, _success_str(tc->success), C_SUCCESS_STR_SIZE_MAX);
                fprintf(stdout, "\t\t%s\n", output);
            }
        }
    }

    fprintf(stdout, "SUMMARY\n");
    fprintf(stdout, "\tSuccesses: %d\n", _testing.success_count);
    fprintf(stdout, "\tFailures: %d\n", _testing.fail_count);

    free(output);
}
