#ifndef SCIEPPEND_CORE_SYMBOL_H
#define SCIEPPEND_CORE_SYMBOL_H

#include "scieppend/core/colour.h"

/* Defines an ascii symbol for the terminal representation.
 * TODO: This will need to move elsewhere at some point as want to support multiple display options.
 */

struct Symbol
{
    char sym;
    struct Colour fg;
    struct Colour bg;
    unsigned int attr;
    unsigned int base_fg_idx;
    unsigned int base_bg_idx;
};

#endif
