#include "scieppend/core/input.h"

#include "scieppend/core/cache_map.h"
#include "scieppend/core/log.h"
#include "scieppend/core/term.h"

#include <string.h>

static const char* C_ARROW_UP            = "\033[A";
static const char* C_ARROW_DOWN          = "\033[B";
static const char* C_ARROW_RIGHT         = "\033[C";
static const char* C_ARROW_LEFT          = "\033[D";
//static const char* C_CTRL_ARROW_UP    = "\003[1;5A";
//static const char* C_CTRL_ARROW_DOWN  = "\003[1;5B";
//static const char* C_CTRL_ARROW_RIGHT = "\003[1;5C";
//static const char* C_CTRL_ARROW_LEFT  = "\003[1;5D";

static struct _InputManager
{
    struct CacheMap pressed;
} _input_manager;

static enum KeyCode _handle_escape_sequence(char buf[8])
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

static enum KeyCode _poll(void)
{
    char buf[8];
    term_getch(buf, 8);
    int buflen = strlen(buf);

    if (buflen == 1)
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

bool input_get_key(enum KeyCode key)
{
    return cache_map_get(&_input_manager.pressed, &key, sizeof(enum KeyCode)) != NULL;
}

void input_poll(void)
{
    term_set_blocking(false);
    cache_map_clear(&_input_manager.pressed);
    enum KeyCode kc = _poll();
    if(kc != KEYCODE_UNKNOWN)
    {
        cache_map_add(&_input_manager.pressed, &kc, sizeof(enum KeyCode), &kc);
    }
    term_set_blocking(true);
}

void input_init(void)
{
    cache_map_init(&_input_manager.pressed, sizeof(enum KeyCode), 4, NULL, NULL);
}

void input_uninit(void)
{
    cache_map_uninit(&_input_manager.pressed);
}
