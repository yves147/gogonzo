#ifndef GONZO_DYNARR_H
#define GONZO_DYNARR_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    uint32_t *data;
    size_t    len;
    size_t    cap;
} dyn_array;

static inline void da_init(dyn_array *a) {
    a->data = NULL;
    a->len  = 0;
    a->cap  = 0;
}

static inline void da_alloc_exact(dyn_array *a, size_t n) {
    a->data = malloc(n * sizeof(uint32_t));
    a->len  = 0;
    a->cap  = n;
}

static inline void da_push(dyn_array *a, uint32_t value) {
    if (a->len == a->cap) {
        a->cap = a->cap ? a->cap * 2 : 4;
        a->data = realloc(a->data, a->cap * sizeof(uint32_t));
    }
    a->data[a->len++] = value;
}

static inline void da_free(dyn_array *a) {
    free(a->data);
    a->data = NULL;
    a->len = a->cap = 0;
}

#endif