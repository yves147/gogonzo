#ifndef GONZO_VALUATION_H
#define GONZO_VALUATION_H

#include <stdint.h>
#include <strings.h>
#include "field.h"

score calculate_position(field *input, uint8_t size)
{
    score black_stones = __builtin_popcountll(input->black.words[0]) + __builtin_popcountll(input->black.words[1]) + __builtin_popcountll(input->black.words[2]);
    score white_stones = __builtin_popcountll(input->white.words[0]) + __builtin_popcountll(input->white.words[1]) + __builtin_popcountll(input->white.words[2]);

    uint64_t black_territory[3] = {0, 0, 0};
    uint64_t white_territory[3] = {0, 0, 0};

    for (uint8_t x = 0; x < size; x++)
    {
        for (uint8_t y = 0; y < size; y++)
        {
            // is already black or white
            if (matrix_at(&input->black, x, y, size) || matrix_at(&input->white, x, y, size)) continue;

            // territory ..
        }
    }
}

#endif
