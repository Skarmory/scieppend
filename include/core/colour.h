#ifndef SCIEPPEND_CORE_COLOUR_H
#define SCIEPPEND_CORE_COLOUR_H

#include <stdbool.h>

#define COL(colour) (&g_colours[(colour)])

// basic colours
#define CLR_BLACK    0
#define CLR_DGREY    1
#define CLR_LGREY    2
#define CLR_WHITE    3

#define CLR_DRED     4
#define CLR_RED      5
#define CLR_LRED     6

#define CLR_DGREEN   7
#define CLR_GREEN    8
#define CLR_LGREEN   9

#define CLR_DYELLOW 10
#define CLR_YELLOW  11
#define CLR_LYELLOW 12

#define CLR_DBLUE   13
#define CLR_BLUE    14
#define CLR_LBLUE   15

#define CLR_DPURPLE 16
#define CLR_PURPLE  17
#define CLR_LPURPLE 18

#define CLR_DCYAN   19
#define CLR_CYAN    20
#define CLR_LCYAN   21

#define CLR_DBROWN  22
#define CLR_BROWN   23
#define CLR_LBROWN  24

#define CLR_DORANGE 25
#define CLR_ORANGE  26
#define CLR_LORANGE 27

#define CLR_DPINK   28
#define CLR_PINK    29
#define CLR_LPINK   30

#define CLR_DEFAULT  31

#define CLR_FOG_OF_WAR 32

// mob difficulty class
// grey - green - blue - purple - yellow - orange - red
// This is more absolute difficulty than relative
// i.e. a grey 'g' could be a 'ghoul' (one of the weakest 'g') but a red 'g' could be an 'infectious ghoul' (one of the strongest 'g')
#define CLR_DC_1 CLR_LGREY
#define CLR_DC_2 CLR_LGREEN
#define CLR_DC_3 CLR_BLUE
#define CLR_DC_4 CLR_YELLOW
#define CLR_DC_5 CLR_ORANGE
#define CLR_DC_6 CLR_LRED
#define CLR_DC_7 CLR_LPURPLE

// No colour. Specify for things that should not be seen (e.g. bare fists as a weapon shouldn't be as a symbol)
#define CLR_NONE CLR_BLACK

// Objects
#define CLR_METAL CLR_WHITE
#define CLR_WOOD  CLR_BROWN

struct Colour
{
    int r;
    int g;
    int b;
};

/* Check if two colours are exactly the same.
 */
bool colour_equal(struct Colour* c1, struct Colour* c2);

/* Check if two colours are similar.
 * See for details: https://www.compuphase.com/cmetric.htm
 */
bool colour_similar(struct Colour* c1, struct Colour* c2);

extern struct Colour g_colours[];

#endif
