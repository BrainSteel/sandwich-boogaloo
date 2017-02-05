
// Random number generator.
// Reference: https://en.wikipedia.org/wiki/Xorshift

#ifndef XORSHIFTSTAR_H
#define XORSHIFTSTAR_H

#include "stdint.h"

// Returns a random integer in the range [0, 2^64)
uint64_t xorshift64star( );

// Returns a random integer in the range [0, range)
uint64_t xorshift64star_uniform( uint64_t range );

// Returns a random integer in the range [low, high];
int64_t xorshift64star_range( int64_t low, int64_t high );

// Returns a random float in the range [0, max)
float xorshift64star_float( float max );

// Seeds the random number generator.
void xorshiftseed( uint64_t seed );

#endif
