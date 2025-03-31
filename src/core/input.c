#include "scieppend/core/input.h"

#include "scieppend/core/cache_map.h"
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

static void _handle_escape_sequence(char buf[8])
{
    if(strcmp(buf, C_ARROW_UP) == 0)
    {
    }

    if(strcmp(buf, C_ARROW_DOWN) == 0)
    {
    }

    if(strcmp(buf, C_ARROW_RIGHT) == 0)
    {
    }

    if(strcmp(buf, C_ARROW_LEFT) == 0)
    {
    }
}

bool input_get_key(enum KeyCode key)
{
    return cache_map_get(&_input_manager.pressed, &key, sizeof(enum KeyCode)) != NULL;
}

void input_poll(void)
{
    char buf[8];
    term_getch(buf, 8);

    int buflen = strlen(buf);

    if(buflen == 1)
    {
        enum KeyCode key = buf[0];
        cache_map_add(&_input_manager.pressed, &key, sizeof(enum KeyCode), &key);
    }
    else
    {
        // If buffer length is > 1 from a single char, it must be an escape key or special character of some sort
        //return _handle_escape_sequence(buf) == key;
    }
}

void input_init(void)
{
    cache_map_init(&_input_manager.pressed, sizeof(enum KeyCode), 4, NULL, NULL);
}

void input_uninit(void)
{
    cache_map_uninit(&_input_manager.pressed);
}
