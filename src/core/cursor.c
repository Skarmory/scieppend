#include "scieppend/core/cursor.h"

void cursor_get_offset(enum KeyCode input, int* x_off, int* y_off)
{
    switch(input)
    {
        case KEYCODE_h:
        {
            *x_off = -1;
            *y_off = 0;
            break;
        }

        case KEYCODE_j:
        {
            *x_off = 0;
            *y_off = 1;
            break;
        }

        case KEYCODE_k:
        {
            *x_off = 0;
            *y_off = -1;
            break;
        }

        case KEYCODE_l:
        {
            *x_off = 1;
            *y_off = 0;
            break;
        }

        case KEYCODE_b:
        {
            *x_off = -1;
            *y_off = 1;
            break;
        }

        case KEYCODE_n:
        {
            *x_off = 1;
            *y_off = 1;
            break;
        }

        case KEYCODE_y:
        {
            *x_off = -1;
            *y_off = -1;
            break;
        }

        case KEYCODE_u:
        {
            *x_off = 1;
            *y_off = 1;
            break;
        }
        default:
    }
}
