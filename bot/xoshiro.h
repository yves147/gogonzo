#ifndef GONZO_XOSHIRO_H
#define GONZO_XOSHIRO_H

#include <stdint.h>
#include "field.h"
#include "splitmix64.h"

static inline uint64_t rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static inline uint64_t xoshiro256ss_next(init_seed *state)
{
    uint64_t *s = state->words;
    const uint64_t result = rotl(s[1] * 5, 7) * 9;

    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 45);

    return result;
}

#endif