#ifndef GONZO_HISTORY_H
#define GONZO_HISTORY_H

#define MAX_GAME_LENGTH 4096

#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#include "splitmix64.h"
#include "dynarray.h"

// PLAN
// die abfrage hier ist (sowie zz geplant) NUR für tromp taylor ko lookup wichtig
// es ist sehr wahrscheinlich, dass die hashmap alleine hier schon zu viel arbeit macht, da man mit einer fehlerrate von 97%+ rechnen kann
// also nutze ich einen Bloom Filter https://en.wikipedia.org/wiki/Bloom_filter
// Rechnung:
// Man kann sehr konservativ bei einen 13x13 Spiel mit durschnittlich 400 Elementen im Bloom Filter rechnen (also 400 gespielten Zügen)
// p = (1 - e ^ ((k * n) / m)) ^ k < 5% mit guten Design Parametern

// n=400, m=4096, 512 byte, k=6 => p=0,76%
typedef struct
{
    uint64_t bloom[64]; // 512 byte
    dyn_array data;
    atomic_size_t *refcount; // != NULL solange data.data mit anderen branches geteilt ist
    bool cow;
} history_filter;

static inline void history_init(history_filter *history)
{
    memset(history->bloom, 0, sizeof(history->bloom));
    da_init(&history->data);
    history->refcount = NULL;
    history->cow = false;
}

// erzeugt in `child` einen neuen branch von `parent`; bloom wird kopiert (relativ billig)
// CoW
static inline void history_branch(history_filter *parent, history_filter *child)
{
    memcpy(child->bloom, parent->bloom, sizeof(parent->bloom));

    if (parent->refcount == NULL)
    {
        parent->refcount = malloc(sizeof(atomic_size_t));
        atomic_init(parent->refcount, 1);
        parent->cow = true;
    }

    atomic_fetch_add(parent->refcount, 1);

    child->data = parent->data;
    child->refcount = parent->refcount;
    child->cow = true;
}

static inline void history_add(history_filter *history, uint64_t el)
{
    if (history->cow && atomic_load(history->refcount) > 1)
    {
        dyn_array copy;
        da_alloc_exact(&copy, history->data.cap ? history->data.cap : 4);
        memcpy(copy.data, history->data.data, history->data.len * sizeof(uint64_t));
        copy.len = history->data.len;

        atomic_fetch_sub(history->refcount, 1);

        history->data = copy;
        history->refcount = NULL;
        history->cow = false;
    }

    uint64_t h2 = splitmix64_mix(el);
    for (int i = 0; i < 6; i++)
    {
        uint64_t idx = (el + i * h2) & 0xFFF;
        uint64_t word_index = idx & 0x3F;
        uint64_t bit_index = (idx >> 6) & 0x3F;
        history->bloom[word_index] |= 1ULL << bit_index;
    }

    da_push(&history->data, el);
}

// find out wether position already existed in this branch of history
// for the positional superko rule of tromp taylor
static inline bool history_find(history_filter *history, uint64_t el)
{
    uint64_t h2 = splitmix64_mix(el);
    for (int i = 0; i < 6; i++)
    {
        uint64_t idx = (el + i * h2) & 0xFFF;
        uint64_t word_index = idx & 0x3F;
        uint64_t bit_index = (idx >> 6) & 0x3F;

        if ((history->bloom[word_index] & (1ULL << bit_index)) == 0)
        {
            return false;
        }
    }

    // bloom filter gibt uns ein ja
    // jetzt müssen wir verifizieren, dass es sich nicht um ein false positive handelt
    return da_has(&history->data, el);
}

static inline void history_clear(history_filter *history)
{
    if (history->refcount != NULL)
    {
        if (atomic_fetch_sub(history->refcount, 1) == 1)
        {
            da_free(&history->data);
            free(history->refcount);
        }
    }
    else
    {
        da_free(&history->data);
    }

    memset(history->bloom, 0, sizeof(history->bloom));
    da_init(&history->data);
    history->refcount = NULL;
    history->cow = false;
}

#endif
