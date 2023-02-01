#include "scieppend/core/screen.h"

#include <stddef.h>

struct Screen
{
    int extent_w;
    int extent_h;
};

struct Screen g_screen;

void screen_get_extents(int* out_w, int* out_h)
{
    *out_w = g_screen.extent_w;
    *out_h = g_screen.extent_h;
}

void screen_set_extents(int w, int h)
{
    g_screen.extent_w = w;
    g_screen.extent_h = h;
}
