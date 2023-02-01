#ifndef SCIEPPEND_CORE_POISSON_DISK_H
#define SCIEPPEND_CORE_POISSON_DISK_H

struct List;
struct RNG;

/* Creates a poisson disk sample of points.
 * Returns a list of struct Point (see: core/geom.h)
 */
struct List* poisson_disk(int width, int height, struct RNG* rng);

#endif
