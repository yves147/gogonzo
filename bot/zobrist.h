#ifndef GONZO_ZOBRIST_H
#define GONZO_ZOBRIST_H

#include <stdint.h>

#include "field.h"
#include "xoshiro.h"

extern uint64_t zobrist_table[MAX_SIZE][MAX_SIZE][2];

static inline void zobrist_init(init_seed *seed)
{
    for (uint8_t x = 0; x < MAX_SIZE; x++)
    {
        for (uint8_t y = 0; y < MAX_SIZE; y++)
        {
            zobrist_table[x][y][0] = xoshiro256ss_next(seed);
            zobrist_table[x][y][1] = xoshiro256ss_next(seed);
        }
    }
}

static inline uint64_t zobrist_move(uint64_t input, uint8_t x, uint8_t y, uint8_t color)
{
    return input ^ zobrist_table[x][y][color];
}

#endif
