#include "core/poisson_disk.h"

#include "core/geom.h"
#include "core/log.h"
#include "core/rng.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const float C_PI = 3.14159265358979323846f;

// STRUCTS

struct PoissonDiskGrid
{
    struct Rect dimensions;
    bool* occupancy;
};

// INTERNAL FUNCS

// Get a random point around given point within the donut shape defined by min and max radius.
static struct Point _random_point_around(struct Point* p, float min_radius, float max_radius, struct RNG* rng)
{
    float r1 = rng_get_float(rng);
    float r2 = rng_get_float(rng);
    float radius = min_radius + ((max_radius - min_radius) * r1);
    float rang = 2.0f * C_PI * r2;

    int nx = (int)((float)p->x + radius * cos(rang));
    int ny = (int)((float)p->y + radius * sin(rang));

    struct Point ret = { .x = nx, .y = ny };
    return ret;
}

// Instead of checking a point against every accepted point to see if there's a collision
// we instead check a new point against the occupancy grid. If any of the grid points that
// lay within the radius of the new disk are set to true, then there is a collision.
// TODO: Need to figure out whether this is actually more efficient.
//       Checking against points would be likely be faster with some spatial partitioning
static bool _point_collides(struct Point* p, struct PoissonDiskGrid* grid, float min_radius)
{
    int irad = (int)min_radius;
    int irad2 = irad * irad;

    for(int x = (p->x - irad); x <= (p->x + irad); ++x)
    for(int y = (p->y - irad); y <= (p->y + irad); ++y)
    {
        struct Point tmp = { .x = x, .y = y };

        if(geom_point_in_rect2(&tmp, &grid->dimensions) && // Make sure the point is within the global dimensions
           geom_distance2_point_point(p, &tmp) < irad2 &&  // We're looping over a square area, so some points will not be within the circle
           grid->occupancy[tmp.x + (tmp.y * grid->dimensions.w)] == true) // Check occupancy
        {
            return true;
        }
    }

    return false;
}

// Sets the "occupancy" around a non-colliding point for quick look up.
static void _set_occupancy_around(struct Point* p, struct PoissonDiskGrid* grid, float min_radius)
{
    int irad = (int)min_radius;
    int irad2 = irad * irad;

    for(int x = (p->x - irad); x <= (p->x + irad); ++x)
    for(int y = (p->y - irad); y <= (p->y + irad); ++y)
    {
        struct Point tmp = { .x = x, .y = y };
        if(geom_point_in_rect2(&tmp, &grid->dimensions) && 
           geom_distance2_point_point(p, &tmp) < irad2)
        {
            grid->occupancy[tmp.x + (tmp.y * grid->dimensions.w)] = true;
        }
    }
}

[[maybe_unused]] static void _debug_print_occupancy([[maybe_unused]] struct PoissonDiskGrid* grid)
{
#ifdef DEBUG_CORE_POISSON_DISK
    char buf[257];
    memset(buf, 0, sizeof(buf));

    for(int y = 0; y < grid->dimensions.h; ++y)
    {
        for(int x = 0; x < grid->dimensions.w; ++x)
        {
            if(grid->occupancy[x + y * grid->dimensions.w] == true)
            {
                buf[x] = 'X';
            }
            else
            {
                buf[x] = 'O';
            }
        }

        log_msg(LOG_DEBUG, buf);
        memset(buf, 0, sizeof(buf));
    }
#endif
}

// EXTERNAL FUNCS

struct List* poisson_disk(int width, int height, struct RNG* rng)
{
    const int new_points_try = 30;
    const float min_radius = 3.0f;
    const float max_radius = 6.0f;

    struct PoissonDiskGrid poisson_disk_grid =
    {
        .dimensions = { .x = 0, .y = 0, .w = width, .h = height },
        .occupancy = calloc(width * height, sizeof(bool))
    };

    struct List* ret_list = list_new();
    struct List  proc_list;
    list_init(&proc_list);

    struct Point ip = { .x = rng_get(rng) % width, .y = rng_get(rng) % height };
    list_add(&proc_list, &ip);

    int add = 0;

    while(!list_empty(&proc_list))
    {
        int ridx = rng_get(rng) % proc_list.count;

        struct Point* p = list_pop_at(&proc_list, ridx);

        for(int i = 0; i < new_points_try; ++i)
        {
            struct Point np = _random_point_around(p, min_radius, max_radius, rng);

            if(geom_point_in_rect2(&np, &poisson_disk_grid.dimensions) &&
               !_point_collides(&np, &poisson_disk_grid, min_radius))
            {
                struct Point* npalloc = malloc(sizeof(struct Point));
                npalloc->x = np.x;
                npalloc->y = np.y;

                list_add(&proc_list, npalloc);
                list_add(ret_list, npalloc);
                _set_occupancy_around(npalloc, &poisson_disk_grid, min_radius);
                ++add;
            }
        }
    }

    free(poisson_disk_grid.occupancy);
    list_uninit(&proc_list);
    return ret_list;
}
