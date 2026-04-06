#ifndef RNG_H
#define RNG_H

void rng_seed(unsigned int seed);
unsigned int rng_next();
float rng_float();
float rng_normal();

#endif
