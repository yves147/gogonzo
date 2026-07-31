#ifndef GONZO_HISTORY_H
#define GONZO_HISTORY_H

#define MAX_GAME_LENGTH 4096
#define HISTORY_CHUNK_SIZE 8 // 64 byte pro Chunk, beste size

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#include "splitmix64.h"
#include "field.h"
#include "bloom.h"

typedef struct history_chunk
{
    struct history_chunk *parent;
    atomic_size_t ref_count;
    uint16_t len;
    uint64_t hashes[HISTORY_CHUNK_SIZE];
} history_chunk_t;

typedef struct
{
    uint64_t bloom[64];    // 512 byte
    history_chunk_t *tail;
    field board;
} history_t;

static inline void history_init(history_t *history)
{
    memset(history->bloom, 0, sizeof(history->bloom));
    history->tail = NULL;

    memset(&history->board, 0, sizeof(history->board));
}

static inline void history_branch(history_t *parent, history_t *child)
{
    memcpy(child->bloom, parent->bloom, sizeof(parent->bloom));
    child->board = parent->board;

    child->tail = parent->tail;
    if (parent->tail != NULL)
    {
        atomic_fetch_add(&parent->tail->ref_count, 1);
    }
}

static inline void history_add(history_t *history, uint64_t el)
{
    if (history->tail == NULL || history->tail->len == HISTORY_CHUNK_SIZE || atomic_load(&history->tail->ref_count) > 1)
    {
        history_chunk_t *chunk = malloc(sizeof(history_chunk_t));
        chunk->parent = history->tail;
        chunk->len = 0;
        atomic_init(&chunk->ref_count, 1);
        history->tail = chunk;
    }

    history->tail->hashes[history->tail->len++] = el;
    bloom_add(history->bloom, el);
}

// find out wether position already existed in this branch of history
// for the positional superko rule of tromp taylor
static inline bool history_find(history_t *history, uint64_t el)
{
    if (!bloom_maybe_has(history->bloom, el))
    {
        return false;
    }

    // bloom filter gibt uns ein ja
    // jetzt müssen wir verifizieren, dass es sich nicht um ein false positive handelt
    for (history_chunk_t *c = history->tail; c != NULL; c = c->parent)
    {
        for (int i = c->len - 1; i >= 0; i--)
        {
            if (c->hashes[i] == el)
            {
                return true;
            }
        }
    }

    return false;
}

static inline void history_clear(history_t *history)
{
    history_chunk_t *cur = history->tail;
    while (cur != NULL)
    {
        history_chunk_t *next = cur->parent;
        if (atomic_fetch_sub(&cur->ref_count, 1) != 1)
        {
            break;
        }
        free(cur);
        cur = next;
    }

    memset(history->bloom, 0, sizeof(history->bloom));
    history->tail = NULL;

    memset(&history->board, 0, sizeof(history->board));
}

#endif
