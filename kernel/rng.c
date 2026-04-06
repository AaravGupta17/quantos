#include "rng.h"

// Xorshift32 — fast, good enough for Monte Carlo
static unsigned int state = 123456789;

void rng_seed(unsigned int seed) {
    state = seed;
}

unsigned int rng_next() {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

// Returns float in [0, 1)
float rng_float() {
    return (float)(rng_next() & 0x7FFFFFFF) / (float)0x7FFFFFFF;
}

// Box-Muller transform — converts uniform to normal distribution
// This is what Monte Carlo needs — normally distributed returns
float rng_normal() {
    float u1 = rng_float();
    float u2 = rng_float();

    // avoid log(0)
    if (u1 < 1e-7f) u1 = 1e-7f;

    // sqrt and log implemented manually — no libc
    // ln approximation using identity: ln(x) = 2*atanh((x-1)/(x+1))
    // simplified for our range using Taylor series
    float x = (u1 - 1.0f) / (u1 + 1.0f);
    float x2 = x * x;
    float ln_u1 = 2.0f * x * (1.0f + x2/3.0f + x2*x2/5.0f + x2*x2*x2/7.0f);
    ln_u1 = -2.0f * (-ln_u1);

    // sqrt approximation using Newton-Raphson
    float val = -2.0f * ln_u1;
    float sqrt_val = val;
    for (int i = 0; i < 10; i++) {
        sqrt_val = 0.5f * (sqrt_val + val / sqrt_val);
    }

    // cos approximation using Taylor series
    float angle = 6.28318f * u2;
    float cos_val = 1.0f - angle*angle/2.0f + angle*angle*angle*angle/24.0f;

    return sqrt_val * cos_val;
}
