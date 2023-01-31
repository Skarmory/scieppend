#ifndef SCIEPPEND_CORE_MATH_UTILS_H
#define SCIEPPEND_CORE_MATH_UTILS_H

#define minu(a, b) ( (a) > (b) ? b : a )
#define maxu(a, b) ( (a) > (b) ? a : b )

/* Clamp an integer value between given low and high values.
 */
int   clamp(int val, int low, int high);

/* Compute log of x to a given base.
 */
float log_base(int x, int base);


/* Map value from one range to another.
 * Value is transformed to a proportion of the old range and that proportion is figured out for the
 * new range. 
 * e.g. map 5.0 in range (0.0 - 10.0) to range (30.0 to 50.0f) = 40.0f
 */
float map_range(float value, float old_low, float old_high, float new_low, float new_high);

// Transform (x, y) from coordinate system with origin (old_ox, old_oy) to new origin (new_ox, new_oy)
// e.g. old = (4, 7)
//      new = (8, 3)
//      in  = (2, 2)
//      output = (2 - 4 + 8, 2 - 7 + 3) = (6, -2)
void math_change_basis(int x, int y, int old_ox, int old_oy, int new_ox, int new_oy, int* x_out, int* y_out);

#endif
