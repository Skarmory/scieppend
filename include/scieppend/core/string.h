#ifndef SCIEPPEND_CORE_STRING_H
#define SCIEPPEND_CORE_STRING_H

/* String implementation.
 */

#include <stddef.h>

struct string
{
    char* buffer;
    int   size;
};

/* Create a string with the given characters.
 */
struct string* string_new(const char* initial);

/* Free the given string.
 */
void string_free(struct string* str);

/* Initialise a string.
 * Useful for strings allocated on the stack.
 */
void string_init(struct string* str, const char* initial);

/* Frees the string internals but does not free the string itself.
 */
void string_uninit(struct string* str);

/* Get the size of the string in characters used.
 */
int string_size(struct string* str);

/* Wrapper to quickly printf a string.
 */
void string_printf(struct string* str);

/* Reverse find index from starting point.
 * Iterates backwards over the string from the given start index and returns the start index of the
 * first occurence of a given "needle".
 */
int string_rfindi(struct string* str, const char needle, size_t start);

void string_set(struct string* str, const char* buffer);

/* Expand the format and set the string.
 */
void string_format(struct string* str, const char* format, ...);

/* Hash the string
 */
int string_hash(const struct string* str);

#endif
