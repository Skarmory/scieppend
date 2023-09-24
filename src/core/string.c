#include "scieppend/core/string.h"

#include "scieppend/core/hash.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// EXTERNAL FUNCS

struct string* string_new(const char* initial)
{
    struct string* new_str = malloc(sizeof(struct string));
    string_init(new_str, initial);
    return new_str;
}

void string_free(struct string* str)
{
    string_uninit(str);
    free(str);
}

void string_init(struct string* str, const char* initial)
{
    str->size = strlen(initial);
    str->buffer = malloc(str->size + 1);
    snprintf(str->buffer, str->size + 1, "%s", initial);
}

void string_uninit(struct string* str)
{
    str->size = 0;
    free(str->buffer);
}

int string_size(struct string* str)
{
    return str->size;
}

void string_printf(struct string* str)
{
    printf(str->buffer);
}

int string_rfindi(struct string* str, const char needle, size_t start)
{
    const char* haystack = str->buffer;

    if(strlen(haystack) < start)
    {
        return -1;
    }

    const char* curr = haystack + start;

    do
    {
        if(*curr == needle)
        {
            return (int)(curr - haystack) + 1;
        }

        --curr;
    }
    while(curr >= haystack);

    return -1;
}

void string_set(struct string* str, const char* value)
{
    int value_len = strlen(value);
    if(value_len > str->size)
    {
        str->buffer = realloc(str->buffer, value_len+1);
        str->size = value_len;
    }

    snprintf(str->buffer, str->size, "%s", value);
}

void string_format(struct string* str, const char* format, ...)
{
    va_list args;
    va_list args_copy;
    va_copy(args_copy, args);

    va_start(args_copy, format);
    int buffer_size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if(buffer_size > str->size)
    {
        str->buffer = realloc(str->buffer, buffer_size + 1);
        str->size = buffer_size;
    }

    va_start(args, format);
    vsnprintf(str->buffer, buffer_size, format, args);
    va_end(args);
}

int string_hash(const struct string* str)
{
    int hashed = hash(str->buffer, str->size);
    return hashed;
}
