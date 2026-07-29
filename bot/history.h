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

typedef struct
{
    //...
    pthread_rwlock_t lock;
} history_hashset;

inline void history_add(history_hashset *history, uint64_t el)
{
    pthread_rwlock_wrlock(&history->lock);
    //...
    pthread_rwlock_unlock(&history->lock);
}

inline void history_find(history_hashset *history, uint64_t el)
{
    pthread_rwlock_rdlock(&history->lock);
    //...
    pthread_rwlock_unlock(&history->lock);
}

inline void history_clear(history_hashset *history)
{
    pthread_rwlock_wrlock(&history->lock);
    //...
    pthread_rwlock_unlock(&history->lock);
}

#endif