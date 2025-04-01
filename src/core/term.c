#include "scieppend/core/term.h"

#include "scieppend/core/bit_flags.h"
#include "scieppend/core/colour.h"
#include "scieppend/core/log.h"

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

// CONSTS

#define C_PRINT_BUFFER_SIZE (1024 * 1024)

#define C_ESCAPE "\033"
#define C_ESCAPE_KIND "["
#define C_GRAPHICS_MODE "m"
#define C_SEP ";"
#define C_HIGH "h"
#define C_LOW "l"

#define C_ALT_BUFFER     C_ESCAPE C_ESCAPE_KIND "?1049"
#define C_ALT_BUFFER_ON  C_ALT_BUFFER C_HIGH
#define C_ALT_BUFFER_OFF C_ALT_BUFFER C_LOW
#define C_CLEAR          C_ESCAPE C_ESCAPE_KIND "2J"
#define C_CURSOR         C_ESCAPE C_ESCAPE_KIND "?25"
#define C_CURSOR_ON      C_CURSOR C_HIGH
#define C_CURSOR_OFF     C_CURSOR C_LOW
#define C_MOVE_FORMAT    C_ESCAPE C_ESCAPE_KIND "%d" C_SEP "%df"

#define C_FOREGROUND "38"
#define C_BACKGROUND "48"
#define C_DEFAULT_FOREGROUND "39"
#define C_DEFAULT_BACKGROUND "49"
#define C_TRUE_COLOUR "2"
#define C_NO_TRUE_COLOUR "5"
#define C_RGB_FORMAT "%d" C_SEP "%d" C_SEP "%d"
#define C_COLOUR(ground) ground C_SEP C_TRUE_COLOUR C_SEP C_RGB_FORMAT
#define C_COLOUR_FG_FORMAT C_COLOUR(C_FOREGROUND)
#define C_COLOUR_BG_FORMAT C_COLOUR(C_BACKGROUND)
#define C_COLOUR_DEFAULT_FG C_DEFAULT_FOREGROUND
#define C_COLOUR_DEFAULT_BG C_DEFAULT_BACKGROUND

#define C_ATTRIBUTE_FORMAT "%d"
#define C_DEFAULT C_ESCAPE C_ESCAPE_KIND "0" C_GRAPHICS_MODE

// STRUCTS

enum TextAttribute
{
    A_NONE       = 0,
    A_BOLD       = 1,
    A_UNDERSCORE = 4,
    A_BLINK      = 5,
    A_REVERSE    = 7
};

struct WriteBuffer
{
    char buffer[C_PRINT_BUFFER_SIZE];
    int buffer_len;
} write_buffer;

struct VTermSymbol
{
    struct Colour fg;
    struct Colour bg;
    TextAttributeFlags ta_flags;
    char symbol;
    bool redraw;
};

struct VTerm
{
    int width;
    int height;
    struct VTermSymbol* symbols;
    struct termios initial_state;
    bool cursor;
};

// INTERNAL VARS

static struct VTerm* vterm = NULL;
static struct Colour C_DEFAULT_colour = { -1, -1, -1 };

// INTERNAL FUNCS

static void _writef(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vsnprintf(write_buffer.buffer + write_buffer.buffer_len, C_PRINT_BUFFER_SIZE - write_buffer.buffer_len, format, args);

    va_end(args);

    int add_len = strlen(write_buffer.buffer + write_buffer.buffer_len);
#ifdef DEBUG_LOG_TERMINAL
    log_format_msg(LOG_DEBUG, "\t%s", write_buffer.buffer + write_buffer.buffer_len);
#endif
    write_buffer.buffer_len += add_len;
}

static void _write_attribute(enum TextAttribute attribute, bool semicolon)
{
    if(semicolon)
    {
        _writef(C_SEP);
    }

    _writef(C_ATTRIBUTE_FORMAT, attribute);
}

static void _write_fg_colour(struct Colour* fg, bool semicolon)
{
    if(semicolon)
    {
        _writef(C_SEP);
    }

    if(fg->r == -1)
    {
        _writef(C_DEFAULT_FOREGROUND);
    }
    else
    {
        _writef(C_COLOUR_FG_FORMAT, fg->r, fg->g, fg->b);
    }
}

static void _write_bg_colour(struct Colour* bg, bool semicolon)
{
    if(semicolon)
    {
        _writef(C_SEP);
    }

    if(bg->r == -1)
    {
        _writef(C_DEFAULT_BACKGROUND);
    }
    else
    {
        _writef(C_COLOUR_BG_FORMAT, bg->r, bg->g, bg->b);
    }
}

static void _flush(void)
{
    write(1, write_buffer.buffer, write_buffer.buffer_len);
    write_buffer.buffer_len = 0;
}

static inline struct VTermSymbol* _term_get_symbol(int x, int y)
{
    return &vterm->symbols[y * vterm->width + x];
}

static inline bool _should_redraw(struct VTermSymbol* symbol, char new_symbol, struct Colour* new_fg, struct Colour* new_bg, TextAttributeFlags new_ta_flags)
{
    if(symbol->symbol != new_symbol)
    {
        return true;
    }

    if(new_fg && !colour_equal(&symbol->fg, new_fg))
    {
        return true;
    }

    if(new_bg && !colour_equal(&symbol->bg, new_bg))
    {
        return true;
    }

    if(symbol->ta_flags != new_ta_flags)
    {
        return true;
    }

    return false;
}

// EXTERNAL FUNCS

void term_init(void)
{
    if(vterm)
    {
        return;
    }

    // Take control of the terminal
    setvbuf(stdout, NULL, _IONBF, 0);
    write(1, C_ALT_BUFFER_ON, sizeof(C_ALT_BUFFER_ON));

    vterm = malloc(sizeof(struct VTerm));
    memset(vterm, 0, sizeof(struct VTerm));

    struct termios t;
    tcgetattr(1, &t);
    vterm->initial_state = t;

    t.c_lflag &= (~ECHO & ~ICANON);
    tcsetattr(1, TCSANOW, &t);
    term_set_blocking(false);

    term_resize();
    term_clear();
    term_set_cursor(false);
    write_buffer.buffer_len = 0;

    atexit(&term_uninit);
}

void term_uninit(void)
{
    if(!vterm)
    {
        return;
    }

    term_clear();
    term_set_cursor(true);

    write(1, C_ALT_BUFFER_OFF, sizeof(C_ALT_BUFFER_OFF));
    tcsetattr(1, TCSANOW, &vterm->initial_state);

    free(vterm->symbols);
    free(vterm);

    vterm = NULL;
}

void term_get_wh(int* w, int* h)
{
    if(w) *w = vterm->width;
    if(h) *h = vterm->height;
}

void term_set_cursor(bool on)
{
    vterm->cursor = on;

    if(on)
    {
        write(1, C_CURSOR_ON, sizeof(C_CURSOR_ON));
    }
    else
    {
        write(1, C_CURSOR_OFF, sizeof(C_CURSOR_OFF));
    }
}

void term_set_echo(bool state)
{
    struct termios t;
    t.c_lflag = state ? ECHO : ~ECHO;
    tcsetattr(1, TCSANOW, &t);
}

void term_set_canon(bool state)
{
    struct termios t;
    t.c_lflag = state ? ICANON : ~ICANON;
    tcsetattr(1, TCSANOW, &t);
}

void term_set_blocking(bool state)
{
    int flags = fcntl(0 , F_GETFL, 0);
    fcntl(0, F_SETFL, flags | (state ? ~O_NONBLOCK : O_NONBLOCK));
}

void term_set_sigint_callback(void(*handler)(int))
{
    signal(SIGINT, handler);
}

void term_resize(void)
{
    struct winsize ws = { 0 };
    ioctl(1, TIOCGWINSZ, &ws);

    vterm->width  = ws.ws_col;
    vterm->height = ws.ws_row;

    struct VTermSymbol* new_sym = realloc(vterm->symbols, vterm->width * vterm->height * sizeof(struct VTermSymbol));
    if(!new_sym)
    {
        // Couldn't realloc the memory, which means something bad has happened.
        // Cannot really continue from this situation so just die.
        abort();
    }

    memset(new_sym, 0, vterm->width * vterm->height * sizeof(struct VTermSymbol));
    vterm->symbols = new_sym;
}

void term_clear(void)
{
    memset(vterm->symbols, 0, vterm->width * vterm->height * sizeof(struct VTermSymbol));
    _writef("%s", C_CLEAR);
}

void term_clear_area(int x, int y, int w, int h)
{
    struct VTermSymbol* symbol;
    for(int _y = y; _y < (y + h); ++_y)
    for(int _x = x; _x < (x + w); ++_x)
    {
        symbol = _term_get_symbol(_x, _y);
        symbol->symbol = ' ';
        symbol->fg = C_DEFAULT_colour;
        symbol->bg = C_DEFAULT_colour;
        symbol->ta_flags = A_NONE;
        symbol->redraw = true;
    }
}

void term_refresh(void)
{
    //TODO: See if batching is possible for fewer ANSI codes.
    struct VTermSymbol* sym = NULL;
    int lx = -1;
    int ly = -1;

    for(int y = 0; y < vterm->height; ++y)
    for(int x = 0; x < vterm->width; ++x)
    {
        sym = _term_get_symbol(x, y);

        if(!sym->redraw)
        {
            continue;
        }

        // Move to
        if (x != lx + 1 || y != ly)
        {
            term_move_cursor(x, y);
        }

        lx = x;
        ly = y;

        // Start graphics mode
        bool semicolon = false;
        _writef(C_ESCAPE);
        _writef(C_ESCAPE_KIND);

        // Set attributes
        if(sym->ta_flags != A_NONE_BIT)
        {
            if(sym->ta_flags & A_BOLD_BIT)
            {
                _write_attribute(A_BOLD, semicolon);
                semicolon = true;
            }

            if(sym->ta_flags & A_UNDERSCORE_BIT)
            {
                _write_attribute(A_UNDERSCORE, semicolon);
                semicolon = true;
            }

            if(sym->ta_flags & A_BLINK_BIT)
            {
                _write_attribute(A_BLINK, semicolon);
                semicolon = true;
            }

            if(sym->ta_flags & A_REVERSE_BIT)
            {
                _write_attribute(A_REVERSE, semicolon);
                semicolon = true;
            }
        }
        else
        {
            _write_attribute(A_NONE, semicolon);
            semicolon = true;
        }

        // Set colours
        _write_fg_colour(&sym->fg, semicolon);
        _write_bg_colour(&sym->bg, semicolon);

        _writef(C_GRAPHICS_MODE);

        // Write symbol
        _writef("%c", sym->symbol);
        _writef(C_DEFAULT);

        sym->redraw = false;
    }

    // Reset term attributes
    _writef(C_DEFAULT);
    _flush();
}

void term_getch(char* buffer_out, int size)
{
    memset(buffer_out, '\0', size);
    read(1, buffer_out, size);
}

void term_wait_on_input(void)
{
    char c;
    read(1, &c, 1);
}

void term_move_cursor(int x, int y)
{
    _writef(C_MOVE_FORMAT, y+1, x+1);
}

void term_set_attr(int x, int y, TextAttributeFlags ta_flags)
{
    struct VTermSymbol* sym = _term_get_symbol(x, y);

    if(!bit_flags_has_flags(sym->ta_flags, ta_flags))
    {
        bit_flags_set_flags(sym->ta_flags, ta_flags);
        sym->redraw = true;
    }
}

void term_unset_attr(int x, int y, TextAttributeFlags ta_flags)
{
    struct VTermSymbol* sym = _term_get_symbol(x, y);

    if(bit_flags_has_flags(sym->ta_flags, ta_flags))
    {
        bit_flags_unset_flags(sym->ta_flags, ta_flags);
        sym->redraw = true;
    }
}

void term_draw_symbol(int x, int y, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, char symbol)
{
    //if(!fg) fg = &C_DEFAULT_COLOUR;
    //if(!bg) bg = &C_DEFAULT_COLOUR;

    struct VTermSymbol* sym = _term_get_symbol(x, y);
    if(_should_redraw(sym, symbol, fg, bg, ta_flags))
    {
        sym->symbol = symbol;
        if(fg) sym->fg = *fg;
        if(bg) sym->bg = *bg;
        sym->ta_flags = ta_flags;
        sym->redraw = true;
    }
}

void term_draw_text(int x, int y, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, const char* text)
{
    int length = strlen(text);
    for(int _x = x; (_x - x) < length; ++_x)
    {
        term_draw_symbol(_x, y, fg, bg, ta_flags, text[_x - x]);
    }
}

void term_draw_ftext(int x, int y, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, const char* format, ...)
{
    va_list args;

    va_start(args, format);

    char tmp_buf[256];
    vsprintf(tmp_buf, format, args);
    term_draw_text(x, y, fg, bg, ta_flags, tmp_buf);

    va_end(args);
}

void term_draw_fntext(int count, int x, int y, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, const char* format, ...)
{
    va_list args;

    va_start(args, format);

    char tmp_buf[256];
    vsnprintf(tmp_buf, count, format, args);
    term_draw_text(x, y, fg, bg, ta_flags, tmp_buf);

    va_end(args);
}

void term_draw_area(int x, int y, int w, int h, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, char symbol)
{
    for(int _y = y; _y < (y+h); ++_y)
    for(int _x = x; _x < (x+w); ++_x)
    {
        term_draw_symbol(_x, _y, fg, bg, ta_flags, symbol);
    }
}
