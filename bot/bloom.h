#ifndef GONZO_BLOOM_H
#define GONZO_BLOOM_H

#include <stdint.h>
#include <stdbool.h>

#include "splitmix64.h"

// PLAN
// die abfrage hier ist (sowie zz geplant) NUR für tromp taylor ko lookup wichtig
// es ist sehr wahrscheinlich, dass die hashmap alleine hier schon zu viel arbeit macht, da man mit einer fehlerrate von 97%+ rechnen kann
// also nutze ich einen Bloom Filter https://en.wikipedia.org/wiki/Bloom_filter
// Rechnung:
// Man kann sehr konservativ bei einen 13x13 Spiel mit durschnittlich 400 Elementen im Bloom Filter rechnen (also 400 gespielten Zügen)
// p = (1 - e ^ ((k * n) / m)) ^ k < 5% mit guten Design Parametern

// n=400, m=4096, 512 byte, k=6 => false hit p=0,76%
static inline void bloom_add(uint64_t bloom[64], uint64_t el)
{
    uint64_t h2 = splitmix64_mix(el);
    for (int i = 0; i < 6; i++)
    {
        uint64_t idx = (el + i * h2) & 0xFFF;
        uint64_t word_index = idx & 0x3F;
        uint64_t bit_index = (idx >> 6) & 0x3F;
        bloom[word_index] |= 1ULL << bit_index;
    }
}

static inline bool bloom_maybe_has(const uint64_t bloom[64], uint64_t el)
{
    uint64_t h2 = splitmix64_mix(el);
    for (int i = 0; i < 6; i++)
    {
        uint64_t idx = (el + i * h2) & 0xFFF;
        uint64_t word_index = idx & 0x3F;
        uint64_t bit_index = (idx >> 6) & 0x3F;

        if ((bloom[word_index] & (1ULL << bit_index)) == 0)
        {
            return false;
        }
    }

    return true;
}

#endif
