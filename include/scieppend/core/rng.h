#ifndef SCIEPPEND_CORE_RNG_H
#define SCIEPPEND_CORE_RNG_H

/* An implementation of a Mersenne twister RNG.
 * See: https://en.wikipedia.org/wiki/Mersenne_Twister
 *
 * Roughly cobbled together based on information I found online.
 * Creates an array of numbers, with each number being based on the previous. The seed value
 * starts the sequence. It does mathsy bit magic and changes the values into nice random ones.
 * It will give out those values sequentially until it reaches the end of the array, where it
 * does the magic again.
 *
 * I don't claim to really know anything at all about how this works or why it is good.
 * I just needed a seedable RNG.
 */

struct RNG;

/* Create a new RNG with given seed.
 */
struct RNG* rng_new(int seed);

/* Free the given RNG.
 */
void rng_free(struct RNG* rng);

/* Get random int.
 */
int rng_get(struct RNG* rng);

/* Get a random int within the bounds of the parameters.
 * Switches max and min around if they are input incorrectly.
 * This will always invoke a number generation.
 */
int rng_range(struct RNG* rng, int min_inclusive, int max_exclusive);

/* Get random float.
 */
float rng_get_float(struct RNG* rng);

#endif
