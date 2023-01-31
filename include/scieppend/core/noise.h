#ifndef SCIEPPEND_CORE_NOISE_H
#define SCIEPPEND_CORE_NOISE_H

/* Generate noise in range [0.0, 1.0] for given coordinates.
 * Higher amplitude biases values towards the minimum and maximum.
 * Higher frequency adds more noise, less smoothness.
 * Octaves are the number of iterations. Amplitude decreases and frequency increases each iteration.
 */
float perlin(float x, float y, float amplitude, float frequency, int octaves);

#endif
