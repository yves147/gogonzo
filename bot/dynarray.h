#ifndef GONZO_DYNARR_H
#define GONZO_DYNARR_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct
{
    uint64_t *data;
    size_t len;
    size_t cap;
} dyn_array;

static inline void da_init(dyn_array *a)
{
    a->data = NULL;
    a->len = 0;
    a->cap = 0;
}

static inline void da_alloc_exact(dyn_array *a, size_t n)
{
    a->data = malloc(n * sizeof(uint64_t));
    a->len = 0;
    a->cap = n;
}

static inline void da_push(dyn_array *a, uint64_t value)
{
    if (a->len == a->cap)
    {
        a->cap = a->cap ? a->cap * 2 : 4;
        a->data = realloc(a->data, a->cap * sizeof(uint64_t));
    }
    a->data[a->len++] = value;
}

// linear search starting from the last few moves
// higher probability to have situational superko in the last moves (1-suicide)
static inline bool da_has(dyn_array *a, uint64_t value)
{
    for (int i = (a->len - 1); i >= 0; i--) {
        if (a->data[i] == value) {
            return true;
        }
    }
    return false;
}

static inline void da_free(dyn_array *a)
{
    free(a->data);
    a->data = NULL;
    a->len = a->cap = 0;
}

#endif