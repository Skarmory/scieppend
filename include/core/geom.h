#ifndef SCIEPPEND_CORE_GEOM_H
#define SCIEPPEND_CORE_GEOM_H

#include "core/list.h"

#include <stdbool.h>

struct Point
{
    int x;
    int y;
};

struct Rect
{
    int x;
    int y;
    int w;
    int h;
};

struct Line
{
    struct List point_list;
};

/* Get the next square along in a line from (x0, y0) to (x1, y1)
 * Puts the next square's (x, y) in the given pointers
 */
bool geom_gen_line_increment(int x0, int y0, int x1, int y1, int* x, int* y, float* err);

/* Create a line from (x0,y0) to (x1,y1) using Bresenham's line algorithm (I think)
 * See: https://www.wikipedia.org/wiki/Bresenham's_line_algorithm
 */
void geom_gen_line(struct Line* out_line, int x0, int y0, int x1, int y1);

/* Check whether (px, py) is inside circle centred at (cx, cy) with radius r
 */
bool geom_point_in_circle(int px, int py, int cx, int cy, int r);

/* Check whether (px, py) is inside rectangle with top left corner at (rx, ry) with width w and height h
 */
bool geom_point_in_rect(int px, int py, int rx, int ry, int w, int h);
bool geom_point_in_rect2(struct Point* p, struct Rect* r);

/* Check whether two axis aligned rectangles intersect.
 */
bool geom_rect_in_rect(int rx0, int ry0, int rw0, int rh0, int rx1, int ry1, int rw1, int rh1);

/* Get distance squared between two points.
 */
float geom_distance2_point_point(struct Point* p1, struct Point* p2);

void geom_debug_log_line(struct Line* line, const char* line_name);

#endif
