#include "scieppend/core/geom.h"

#include "scieppend/core/log.h"
#include <math.h>
#include <stdlib.h>

// INTERNAL FUNCS

/*      
 *      \    |    /
 *       \ 6 | 7 /
 *        \  |  /
 *     5   \ | /   8
 *          \|/ 
 *   ------------------
 *          /|\
 *     4   / | \   1
 *        /  |  \
 *       / 3 | 2 \
 *      /    |    \
 *   
 *   
 *   1: dx > dy, +dx +dy
 *   8: dx > dy, +dx -dy
 *   
 *   2: dy > dx, +dx +dy
 *   7: dy > dx, +dx -dy
 *   
 *   3: dy > dx, -dx +dy
 *   6: dy > dx, -dx -dy
 *   
 *   4: dx > dy, -dx +dy
 *   5: dx > dy, -dx -dy
 */
static inline void _line_increment(int* x, int* y, float* err, float dx, float dy, float derr)
{
    (*x) += (int)((dx > 0.0f) - (dx < 0.0f));
    (*err) += derr;

    if(*err >= 0.5f)
    {
        (*y)   += (int)((dy > 0.0f) - (dy < 0.0f));
        (*err) -= 1.0f;
    }
}

// EXTERNAL FUNCS

bool geom_gen_line_increment(int x0, int y0, int x1, int y1, int* x, int* y, float* err)
{
    if(*x == x1 && *y == y1)
    {
        return false;
    }

    float dx   = (float)x1 - (float)x0;
    float dy   = (float)y1 - (float)y0;
    float derr = 0.5f;

    if(fabs(dx) >= fabs(dy))
    {
        derr = (dx == 0.0f ? 0.5f : fabs(dy / dx));
        _line_increment(x, y, err, dx, dy, derr);
    }
    else
    {
        derr = (dy == 0.0f ? 0.5f : fabs(dx / dy));
        _line_increment(y, x, err, dy, dx, derr);
    }

    return true;
}

void geom_gen_line(struct Line* out_line, int x0, int y0, int x1, int y1)
{
    int nx = x0; // Line segment next coords
    int ny = y0;
    float err = 0.0f;

    list_init(&out_line->point_list);

    struct Point* p = malloc(sizeof(struct Point));
    p->x = x0;
    p->y = y0;

    list_add(&out_line->point_list, p);

    // Set the in-between line segments
    while(geom_gen_line_increment(x0, y0, x1, y1, &nx, &ny, &err))
    {
        p = malloc(sizeof(struct Point));
        p->x = nx;
        p->y = ny;
        list_add(&out_line->point_list, p);
    }
}

bool geom_point_in_circle(int px, int py, int cx, int cy, int r)
{
    int dx = abs(px - cx);
    int dy = abs(py - cy);
    int len = (dx * dx) + (dy * dy);
    return len < (r*r);
}

bool geom_point_in_rect(int px, int py, int rx, int ry, int w, int h)
{
    return (px >= rx && px < (rx + w) && py >= ry && py < (ry + h));
}

bool geom_point_in_rect2(struct Point* p, struct Rect* r)
{
    return geom_point_in_rect(p->x, p->y, r->x, r->y, r->w, r->h);
}

bool geom_rect_in_rect(int rx0, int ry0, int rw0, int rh0, int rx1, int ry1, int rw1, int rh1)
{
    return rx0 > rx1 &&
           ry0 > ry1 &&
           rx0 + rw0 < rx1 + rw1 &&
           ry0 + rh0 < ry1 + rh1;
}

float geom_distance2_point_point(struct Point* p1, struct Point* p2)
{
    float xspan = p1->x - p2->x;
    float yspan = p1->y - p2->y;

    return xspan * xspan + yspan * yspan;
}

void geom_debug_log_line([[maybe_unused]] struct Line* line, [[maybe_unused]] const char* line_name)
{
#ifdef DEBUG_CORE_GEOM
    log_format_msg(LOG_DEBUG, "Debugging line: %s", line_name);
    int seg = 0;
    struct ListNode* n = NULL;
    list_for_each(&line->point_list, n)
    {
        struct Point* cptr = n->data;
        log_format_msg(LOG_DEBUG, "\tsegment %d: (%d, %d)", seg, cptr->x, cptr->y);
        ++seg;
    }
#endif
}
