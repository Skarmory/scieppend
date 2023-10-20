#ifndef SCIEPPEND_CORE_STACK_ARRAY_H
#define SCIEPPEND_CORE_STACK_ARRAY_H

#include <stdio.h>
#include <stdlib.h>

/* Array with static sized contiguous memory directly inside it.
 * Basically a normal array, except with helpful "functions"
 */

#define StackArray(typename, capacity)\
    struct\
    {\
        typename data[(capacity)];\
        int count;\
    }

#define stackarray_init(array)\
    do { (array)->count = 0; } while(0)

#define stackarray_add(array, item)\
    do\
    {\
        if((array)->count == (sizeof (array)->data / sizeof (array)->data[0]) )\
        {\
            fprintf(stderr, "Stack array out of memory");\
            abort();\
        }\
        (array)->data[ (array)->count ] = item;\
        ++((array)->count);\
    } while(0)

#define stackarray_remove(array, index)\
    do\
    {\
        memcpy(&((array)->data[(index)]), &((array)->data[(array)->count-1]), sizeof (array)->data[0]);\
        memset(&((array)->data[(array)->count-1]), '\0', sizeof (array)->data[0]);\
        --((array)->count);\
    } while(0)

#define stackarray_get(array, index)\
    ((array)->data[index])

#define stackarray_set(array, index, item)\
    do\
    {\
        (array)->data[(index)] = item;\
    } while(0)

#define stackarray_clear(array)\
    do\
    {\
        memset((array)->data, '\0', sizeof (array)->data);\
        (array)->count = 0;\
    } while(0)

#endif
