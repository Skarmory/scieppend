#ifndef SCIEPPEND_CORE_STRING_H
#define SCIEPPEND_CORE_STRING_H

/* String implementation.
 */

#include <stddef.h>

typedef struct _string
{
    char* buffer;
    int   size;
} string;

/* Create a string with the given characters.
 */
string* string_new(const char* initial);

/* Free the given string.
 */
void string_free(string* str);

/* Initialise a string.
 * Useful for strings allocated on the stack.
 */
void string_init(string* str, const char* initial);

/* Frees the string internals but does not free the string itself.
 */
void string_uninit(string* str);

/* Get the size of the string in characters used.
 */
int string_size(string* str);

/* Wrapper to quickly printf a string.
 */
void string_printf(string* str);

/* Reverse find index from starting point.
 * Iterates backwards over the string from the given start index and returns the start index of the
 * first occurence of a given "needle".
 */
int string_rfindi(string* str, const char needle, size_t start);

/* Expand the format and set the string.
 */
void string_format(string* str, const char* format, ...);

#endif
