#include "core/input_keycodes.h"

#include "core/term.h"
#include <string.h>

// CONSTS

static const char* C_ARROW_UP            = "\033[A";
static const char* C_ARROW_DOWN          = "\033[B";
static const char* C_ARROW_RIGHT         = "\033[C";
static const char* C_ARROW_LEFT          = "\033[D";
//static const char* C_CTRL_ARROW_UP    = "\003[1;5A";
//static const char* C_CTRL_ARROW_DOWN  = "\003[1;5B";
//static const char* C_CTRL_ARROW_RIGHT = "\003[1;5C";
//static const char* C_CTRL_ARROW_LEFT  = "\003[1;5D";

// INTERNAL FUNCS

enum KeyCode _handle_escape_sequence(char buf[8])
{
    if(strcmp(buf, C_ARROW_UP) == 0)
    {
        return KEYCODE_ARROW_UP;
    }

    if(strcmp(buf, C_ARROW_DOWN) == 0)
    {
        return KEYCODE_ARROW_DOWN;
    }

    if(strcmp(buf, C_ARROW_RIGHT) == 0)
    {
        return KEYCODE_ARROW_RIGHT;
    }

    if(strcmp(buf, C_ARROW_LEFT) == 0)
    {
        return KEYCODE_ARROW_LEFT;
    }

    return KEYCODE_UNKNOWN;
}

// EXTERNAL FUNCS

enum KeyCode get_key(void)
{
    char buf[8];
    term_getch(buf, 8);

    if(strlen(buf) == 1)
    {
        // Simple ASCII code to handle
        if((buf[0] >= KEYCODE_CHAR_RANGE_START && buf[0] <= KEYCODE_CHAR_RANGE_END) || buf[0] == KEYCODE_ENTER || buf[0] == KEYCODE_ESC || buf[0] == KEYCODE_BACKSPACE)
        {
            return buf[0];
        }

        return KEYCODE_UNKNOWN;
    }

    // If buffer length is > 1 from a single char, it must be an escape key or special character of some sort
    return _handle_escape_sequence(buf);
}
