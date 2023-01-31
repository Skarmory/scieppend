#ifndef SCIEPPEND_CORE_CURSOR_H
#define SCIEPPEND_CORE_CURSOR_H

#include "core/input_keycodes.h"

struct Cursor
{
    int x;
    int y;
};

void cursor_get_offset(enum KeyCode input, int* x_off, int* y_off);

#endif
