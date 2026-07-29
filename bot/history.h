#ifndef GONZO_HISTORY_H
#define GONZO_HISTORY_H

#define MAX_GAME_LENGTH 4096

#include <math.h>
#include <stdint.h>
#include <pthread.h>

#include "splitmix64.h"

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
    uint64_t data;
    history_node *before;
} history_node;

typedef struct
{
    uint64_t bloom[64]; // 512 byte
    history_node *tail;
    pthread_rwlock_t lock;
    bool cow;
} history_filter;

inline void history_add(history_filter *history, uint64_t el)
{
    pthread_rwlock_wrlock(&history->lock);

    uint64_t h2 = splitmix64_mix(el);
    for (int i = 0; i < 6; i++)
    {
        uint64_t idx = (el + i * h2) & 0xFFF;
        uint64_t word_index = idx & 0x3F;
        uint64_t bit_index = (idx >> 6) & 0x3F;
        history->bloom[word_index] |= 1ULL << bit_index;
    }

    pthread_rwlock_unlock(&history->lock);
}

inline void history_branch(history_filter history, uint64_t el)
{
    pthread_rwlock_t new_lock;
    history.lock = new_lock;
    history.cow = true;
}

// find out wether position already existed in this branch of history
// for the positional superko rule of tromp taylor
inline bool history_find(history_filter *history, uint64_t el)
{
    pthread_rwlock_rdlock(&history->lock);

    uint64_t h2 = splitmix64_mix(el);
    for (int i = 0; i < 6; i++)
    {
        uint64_t idx = (el + i * h2) & 0xFFF;
        uint64_t word_index = idx & 0x3F;
        uint64_t bit_index = (idx >> 6) & 0x3F;

        if ((history->bloom[word_index] & (1ULL << bit_index)) == 0)
        {
            pthread_rwlock_unlock(&history->lock);
            return false;
        }
    }

    // bloom filter gibt uns ein ja
    // jetzt müssen wir verifizieren, dass es sich nicht um ein false positive handelt
    history_node *tmp = history->tail;
    while (tmp != NULL)
    {
        if (tmp->data == el)
        {
            pthread_rwlock_unlock(&history->lock);
            return true;
        }
        tmp = tmp->before;
    }

    pthread_rwlock_unlock(&history->lock);
    return false;
}

inline void history_clear(history_filter *history)
{
    pthread_rwlock_wrlock(&history->lock);
    //...
    pthread_rwlock_unlock(&history->lock);
}

#endif