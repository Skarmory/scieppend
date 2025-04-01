#ifndef SCIEPPEND_CORE_TERM_H
#define SCIEPPEND_CORE_TERM_H

/* Terminal-based game display functionality.
 * Uses ANSI escape codes to "take over" the terminal and display the game.
 * NOTE: Display does not occur until "term_refresh()" is called!
 *
 * TODO: Move this once we support multiple display options.
 */

#include <stdbool.h>

struct Colour;

enum TextAttributeFlag
{
    A_NONE_BIT       = 0,
    A_BOLD_BIT       = 1 << 0,
    A_UNDERSCORE_BIT = 1 << 1,
    A_BLINK_BIT      = 1 << 2,
    A_REVERSE_BIT    = 1 << 3
};
typedef unsigned int TextAttributeFlags;

/* Initialises the terminal, taking over the altbuffer and setting up internal state.
 * Won't do anything if the terminal has already been initialised.
 */
void term_init(void);

/* Uninitialise the terminal, clean up state, return terminal to previous state (hopefully).
 */
void term_uninit(void);

/* Gets the width and height of the terminals in characters.
 * i.e. how many characters are in each row and column.
 */
void term_get_wh(int* w, int* h);

/* Sets cursor visibility.
 */
void term_set_cursor(bool on);

/* Sets whether to echo input characters to stdout.
 */
void term_set_echo(bool state);

/* Set whether canonical mode is enabled.
 */
void term_set_canon(bool state);

/* Set whether the terminal blocks on read.
 */
void term_set_blocking(bool state);

/* Set sigint callback handler function.
 */
void term_set_sigint_callback(void(*sig)(int));

/* Force resize of terminal state.
 */
void term_resize(void);

/* Clears the entire terminal of display content.
 */
void term_clear(void);

/* Clear a given area of display content.
 */
void term_clear_area(int x, int y, int w, int h);

/* Writes to and flushes the terminal display buffer.
 */
void term_refresh(void);

/* Read into buffer.
 */
void term_getch(char* buffer_out, int size);

/* Blocking input, waits for any input but discards it.
 */
void term_wait_on_input(void);

/* Move the cursor to given x y position.
 */
void term_move_cursor(int x, int y);

/* Set attr flags for given x y position.
 */
void term_set_attr(int x, int y, TextAttributeFlags ta_flags);

/* Unset attr flags for given x y position.
 */
void term_unset_attr(int x, int y, TextAttributeFlags ta_flags);

/* Draw given symbol with given parameters.
 */
void term_draw_symbol(int x, int y, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, char symbol);

/* Draw a string, beginning at x y and extending horizontally, with given parameters.
 */
void term_draw_text(int x, int y, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, const char* text);

/* Expand a format string and draw it with given parameters.
 * Uses vsprintf().
 */
void term_draw_ftext(int x, int y, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, const char* format, ...);

/* Expand a format string and draw it with given parameters.
 * Uses vsnprintf().
 */
void term_draw_fntext(int count, int x, int y, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, const char* format, ...);

/* Draw a filled square area with given parameters.
 */
void term_draw_area(int x, int y, int w, int h, struct Colour* fg, struct Colour* bg, TextAttributeFlags ta_flags, char symbol);

#endif
