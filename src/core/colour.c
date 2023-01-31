#include "core/colour.h"

// INTERNAL CONSTS

#define COLOUR(r, g, b) { r, g, b }

// EXTERNAL FUNCS

bool colour_equal(struct Colour* c1, struct Colour* c2)
{
    return c1->r == c2->r &&
           c1->g == c2->g &&
           c1->b == c2->b;
}

bool colour_similar(struct Colour* c1, struct Colour* c2)
{
    int rmean = (c1->r + c2->r) / 2;
    int r     = c1->r - c2->r;
    int g     = c1->g - c2->g;
    int b     = c1->b - c2->b;
    int dist2 = (((512 + rmean) * r * r) >> 8) + (4 * g * g) + (((767 - rmean) * b * b) >> 8);

    return dist2 < 150000;
}

// EXTERNAL VARS

struct Colour g_colours[] =
{
    COLOUR(  0,   0,   0), // black
    COLOUR( 32,  32,  32), // dark grey
    COLOUR(128, 128, 128), // light grey
    COLOUR(255, 255, 255), // white

    COLOUR( 64,   0,   0), // dark red
    COLOUR(128,   0,   0), // red
    COLOUR(255,   0,   0), // light red

    COLOUR(  0,  64,   0), // dark green
    COLOUR(  0, 128,   0), // green
    COLOUR(  0, 255,   0), // light green

    COLOUR( 64,  64,   0), // dark yellow
    COLOUR(128, 128,   0), // yellow
    COLOUR(255, 255,   0), // light yellow

    COLOUR(  0,   0,  64), // dark blue
    COLOUR(  0,   0, 128), // blue
    COLOUR(  0,   0, 255), // light blue

    COLOUR( 64,   0,  64), // dark purple
    COLOUR(128,   0, 128), // purple
    COLOUR(255,   0, 255), // light purple

    COLOUR(  0,  64,  64), // dark cyan
    COLOUR(  0, 128, 128), // cyan
    COLOUR(  0, 255, 255), // light cyan

    COLOUR(101,  67,  33), // dark brown
    COLOUR(165,  43,  43), // brown
    COLOUR(181, 101,  29), // light brown

    COLOUR(255,  69,   0), // dark orange
    COLOUR(255, 140,   0), // orange
    COLOUR(255, 165,   0), // light orange

    COLOUR(255,  60, 200), // dark pink
    COLOUR(255, 100, 200), // pink
    COLOUR(255, 190, 200),  // light pink

    COLOUR(-1, -1, -1), // default

    COLOUR(64, 64, 64) // fog of war
};
