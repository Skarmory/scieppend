#include "scieppend/core/draw.h"

#include "scieppend/core/log.h"
#include "scieppend/core/screen.h"
#include "scieppend/core/term.h"

#include <signal.h>
#include <stdbool.h>

typedef void(*resize_fp)(void);
typedef void(*get_extents_fp)(int* w, int* h);

struct DrawState
{
    bool resize;
};

struct DrawFuncs
{
    resize_fp      resize;
    get_extents_fp get_extents;
};

static struct DrawState state;
static struct DrawFuncs funcs;

static void _sigwinch_handler([[maybe_unused]] int _)
{
    state.resize = true;

    int w = 0;
    int h = 0;
    funcs.resize();
    funcs.get_extents(&w, &h);
    screen_set_extents(w, h);
    state.resize = false;

    log_format_msg(LOG_DEBUG, "Resizing screen to %dx%d", w, h);
}

void draw_init(void)
{
    state.resize = false;

    //TODO: Eventually handle different drawing, like SDL
    term_init();
    funcs.resize      = &term_resize;
    funcs.get_extents = &term_get_wh;

    signal(SIGWINCH, &_sigwinch_handler);
}

void draw_uninit(void)
{
    //TODO: Eventually handle different drawing, like SDL
    term_uninit();
}

void draw_begin(void)
{
}
