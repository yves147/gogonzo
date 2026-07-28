#ifndef GONZO_FIELD_H
#define GONZO_FIELD_H

#include <stdint.h>
#include <stdbool.h>

typedef int16_t score;

typedef enum COLOR
{
    BLACK = 0,
    WHITE = 1
} COLOR;

typedef struct
{
    uint64_t words[3];
} matrix;

typedef struct
{
    matrix black; // 24
    matrix white; // 24
} field;

uint8_t matrix_at(const matrix *input, uint8_t x, uint8_t y, uint8_t size)
{
    uint8_t index = x * size + y;
    uint8_t part = index / 64;
    index %= 64;
    uint64_t mask = 1ULL << index;
    return (input->words[part] & mask) > 0;
}

bool is_winning(score s, COLOR color, uint8_t komi)
{
    s += komi;
    if (s >= 0)
    {
        return color == WHITE;
    }
    else
    {
        return color == BLACK;
    }
}

#endif