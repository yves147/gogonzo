#ifndef GONZO_FILTER_H
#define GONZO_FILTER_H

#include <stdint.h>
#include "field.h"
#include "permutations.h"

typedef enum FILTER
{
    NONE = 0,
    AND,
    OR,
    XOR
} FILTER;

static inline void field_filter(field *input, field *filter, uint8_t is9, FILTER op_black, FILTER op_white)
{
    uint8_t upper_bound = is9 ? 1 : 2;
    for (uint8_t i = 0; i <= upper_bound; i++)
    {
        switch (op_black)
        {
        case AND:
            input->black.words[i] &= filter->black.words[i];
            break;

        case OR:
            input->black.words[i] |= filter->black.words[i];
            break;

        case XOR:
            input->black.words[i] ^= filter->black.words[i];
            break;

        default:
            break;
        }

        switch (op_white)
        {
        case AND:
            input->white.words[i] &= filter->white.words[i];
            break;

        case OR:
            input->white.words[i] |= filter->white.words[i];

        case XOR:
            input->white.words[i] ^= filter->white.words[i];

        default:
            break;
        }
    }
}

// everything from now on is cool because:
// - no branching
// - only static instructions
// take a look at https://programming.sirrida.de/calcperm.php

// field 9x9 90° rotation
// ~121 Instr., 0 branches
static inline void field_9_rotate_90(uint64_t input[3])
{
    __uint128_t x = ((__uint128_t)input[0] << 64) | (__uint128_t)input[1];
    // Gather/scatter opt, split per 64-bit half since pdep only exists for
    // 64-bit registers: ~2x9 pdeps, 9 mask pairs

    uint64_t m0_hi = 0x0000000000000100ULL, m0_lo = 0x8040201008040201ULL;
    uint64_t m1_hi = 0x0000000000000201ULL, m1_lo = 0x0080402010080402ULL;
    uint64_t m2_hi = 0x0000000000000402ULL, m2_lo = 0x0100804020100804ULL;
    uint64_t m3_hi = 0x0000000000000804ULL, m3_lo = 0x0201008040201008ULL;
    uint64_t m4_hi = 0x0000000000001008ULL, m4_lo = 0x0402010080402010ULL;
    uint64_t m5_hi = 0x0000000000002010ULL, m5_lo = 0x0804020100804020ULL;
    uint64_t m6_hi = 0x0000000000004020ULL, m6_lo = 0x1008040201008040ULL;
    uint64_t m7_hi = 0x0000000000008040ULL, m7_lo = 0x2010080402010080ULL;
    uint64_t m8_hi = 0x0000000000010080ULL, m8_lo = 0x4020100804020100ULL;

    x = pdep128(x >> 72, m0_hi, m0_lo) | pdep128(x >> 63, m1_hi, m1_lo) | pdep128(x >> 54, m2_hi, m2_lo) | pdep128(x >> 45, m3_hi, m3_lo) | pdep128(x >> 36, m4_hi, m4_lo) | pdep128(x >> 27, m5_hi, m5_lo) | pdep128(x >> 18, m6_hi, m6_lo) | pdep128(x >> 9, m7_hi, m7_lo) | pdep128(x, m8_hi, m8_lo);

    input[0] = (uint64_t)(x >> 64);
    input[1] = (uint64_t)x;
}

// field 9x9 180° rotation
// ~87 Instr., 0 branches
static inline void field_9_rotate_180(matrix* input)
{
    __uint128_t x = ((__uint128_t)input->words[0] << 64) | (__uint128_t)input->words[1];
    // bswap + rol 88 + bit group moving: 18 cycles, 8 masks
    x = bswap(x);
    x = rol(x, 88);
    x = (x & U128(0x0000000000010101, 0x0101010101010101)) | ((x & U128(0x0000000000808080, 0x8080808080808000)) >> 14) | ((x & U128(0x0000000000404040, 0x4040404040404000)) >> 12) | ((x & U128(0x0000000000202020, 0x2020202020202000)) >> 10) | ((x & U128(0x0000000000101010, 0x1010101010101000)) >> 8) | ((x & U128(0x0000000000080808, 0x0808080808080800)) >> 6) | ((x & U128(0x0000000000040404, 0x0404040404040400)) >> 4) | ((x & U128(0x0000000000020202, 0x0202020202020200)) >> 2);

    input->words[0] = (uint64_t)(x >> 64);
    input->words[1] = (uint64_t)x;
}

// field 9x9 270°/-90° rotation
// ~128 Instr., 0 branches
static inline void field_9_rotate_270(matrix* input)
{
    __uint128_t x = ((__uint128_t)input->words[0] << 64) | (__uint128_t)input->words[1];
    // Gather/scatter opt, split per 64-bit half since pext only exists for
    // 32/64-bit registers: 18 pexts, 9 mask pairs

    uint64_t m0_hi = 0x0000000000010080ULL, m0_lo = 0x4020100804020100ULL;
    uint64_t m1_hi = 0x0000000000008040ULL, m1_lo = 0x2010080402010080ULL;
    uint64_t m2_hi = 0x0000000000004020ULL, m2_lo = 0x1008040201008040ULL;
    uint64_t m3_hi = 0x0000000000002010ULL, m3_lo = 0x0804020100804020ULL;
    uint64_t m4_hi = 0x0000000000001008ULL, m4_lo = 0x0402010080402010ULL;
    uint64_t m5_hi = 0x0000000000000804ULL, m5_lo = 0x0201008040201008ULL;
    uint64_t m6_hi = 0x0000000000000402ULL, m6_lo = 0x0100804020100804ULL;
    uint64_t m7_hi = 0x0000000000000201ULL, m7_lo = 0x0080402010080402ULL;
    uint64_t m8_hi = 0x0000000000000100ULL, m8_lo = 0x8040201008040201ULL;

    x = (__uint128_t)pext128(x, m0_hi, m0_lo) | ((__uint128_t)pext128(x, m1_hi, m1_lo) << 9) | ((__uint128_t)pext128(x, m2_hi, m2_lo) << 18) | ((__uint128_t)pext128(x, m3_hi, m3_lo) << 27) | ((__uint128_t)pext128(x, m4_hi, m4_lo) << 36) | ((__uint128_t)pext128(x, m5_hi, m5_lo) << 45) | ((__uint128_t)pext128(x, m6_hi, m6_lo) << 54) | ((__uint128_t)pext128(x, m7_hi, m7_lo) << 63) | ((__uint128_t)pext128(x, m8_hi, m8_lo) << 72);

    input->words[0] = (uint64_t)(x >> 64);
    input->words[1] = (uint64_t)x;
}

// field 13x13 90° rotation
static inline void field_13_rotate_90(matrix input)
{
    uint256_t x;
    x.hi = (uint128_t)input.words[0];
    x.lo = ((uint128_t)input.words[1] << 64) | input.words[2];

    uint64_t m0w3 = 0x0007ffffffffffffULL, m0w2 = 0xfffffffff5555555ULL, m0w1 = 0x5555555555555555ULL, m0w0 = 0x5555555555555555ULL;
    uint64_t m1w3 = 0xfff8000000000000ULL, m1w2 = 0x000000000aaaaaaaULL, m1w1 = 0xaaaaaaaaaaaaaaaaULL, m1w0 = 0xaaaaaaaaaaaaaaaaULL;
    uint64_t m2w3 = 0x0000001249249249ULL, m2w2 = 0x7fffffffffffffffULL, m2w1 = 0xffffff5555555555ULL, m2w0 = 0x5555555555555555ULL;
    uint64_t m3w3 = 0xffffffedb6db6db6ULL, m3w2 = 0x8000000000000000ULL, m3w1 = 0x000000aaaaaaaaaaULL, m3w0 = 0xaaaaaaaaaaaaaaaaULL;
    uint64_t m4w3 = 0x0000000000000000ULL, m4w2 = 0x0000009249249249ULL, m4w1 = 0x2492492492a54a95ULL, m4w0 = 0x2a54a952a54a952aULL;
    uint64_t m5w3 = 0xffffffffffffffffULL, m5w2 = 0xffffff6db6db6db6ULL, m5w1 = 0xdb6db6db6d5ab56aULL, m5w0 = 0xd5ab56ad5ab56ad5ULL;
    uint64_t m6w3 = 0xffffffffffffffffULL, m6w2 = 0xfffffea9554aaa55ULL, m6w1 = 0x52aa9554aaa5552aULL, m6w0 = 0xa9554aaa5552aa95ULL;
    uint64_t m7w3 = 0x0000000000000000ULL, m7w2 = 0x00000156aab555aaULL, m7w1 = 0xad556aab555aaad5ULL, m7w0 = 0x56aab555aaad556aULL;

    x = or256(pdep256(shr256(x, 91), m0w3, m0w2, m0w1, m0w0),
              pdep256(x, m1w3, m1w2, m1w1, m1w0));

    x = or256(pdep256(shr256(x, 104), m2w3, m2w2, m2w1, m2w0),
              pdep256(x, m3w3, m3w2, m3w1, m3w0));

    x = or256(pdep256(shr256(x, 191), m4w3, m4w2, m4w1, m4w0),
              pdep256(x, m5w3, m5w2, m5w1, m5w0));

    x = or256(pdep256(shr256(x, 91), m6w3, m6w2, m6w1, m6w0),
              pdep256(x, m7w3, m7w2, m7w1, m7w0));

    input.words[0] = (uint64_t)x.hi;
    input.words[1] = (uint64_t)(x.lo >> 64);
    input.words[2] = (uint64_t)x.lo;
}

// field 13x13 180° rotation
static inline void field_13_rotate_180(matrix input)
{
    uint256_t x;
    x.hi = (uint128_t)input.words[0];
    x.lo = ((uint128_t)input.words[1] << 64) | input.words[2];

    x = bswap256(x);
    x = rol256(x, 176);

    uint256_t m0 = {U128(0x0000000000000000, 0x0000010101010101), U128(0x0101010101010101, 0x0101010101010101)};
    uint256_t m1 = {U128(0x0000000000000000, 0x0000808080808080), U128(0x8080808080808080, 0x8080808080808000)};
    uint256_t m2 = {U128(0x0000000000000000, 0x0000404040404040), U128(0x4040404040404040, 0x4040404040404000)};
    uint256_t m3 = {U128(0x0000000000000000, 0x0000202020202020), U128(0x2020202020202020, 0x2020202020202000)};
    uint256_t m4 = {U128(0x0000000000000000, 0x0000101010101010), U128(0x1010101010101010, 0x1010101010101000)};
    uint256_t m5 = {U128(0x0000000000000000, 0x0000080808080808), U128(0x0808080808080808, 0x0808080808080800)};
    uint256_t m6 = {U128(0x0000000000000000, 0x0000040404040404), U128(0x0404040404040404, 0x0404040404040400)};
    uint256_t m7 = {U128(0x0000000000000000, 0x0000020202020202), U128(0x0202020202020202, 0x0202020202020200)};

    x = or256(and256(x, m0),
              or256(shr256(and256(x, m1), 14),
                    or256(shr256(and256(x, m2), 12),
                          or256(shr256(and256(x, m3), 10),
                                or256(shr256(and256(x, m4), 8),
                                      or256(shr256(and256(x, m5), 6),
                                            or256(shr256(and256(x, m6), 4),
                                                  shr256(and256(x, m7), 2))))))));

    input.words[0] = (uint64_t)x.hi;
    input.words[1] = (uint64_t)(x.lo >> 64);
    input.words[2] = (uint64_t)x.lo;
}

// field 13x13 270°/-90° rotation
static inline void field_13_rotate_270(matrix input)
{
    uint256_t x;
    x.hi = (uint128_t)input.words[0];
    x.lo = ((uint128_t)input.words[1] << 64) | input.words[2];

    uint64_t m0w3 = 0xffffffffffffffffULL, m0w2 = 0xfffffea9554aaa55ULL, m0w1 = 0x52aa9554aaa5552aULL, m0w0 = 0xa9554aaa5552aa95ULL;
    uint64_t m1w3 = 0x0000000000000000ULL, m1w2 = 0x00000156aab555aaULL, m1w1 = 0xad556aab555aaad5ULL, m1w0 = 0x56aab555aaad556aULL;
    uint64_t m2w3 = 0x0000000000000000ULL, m2w2 = 0x0000009249249249ULL, m2w1 = 0x2492492492a54a95ULL, m2w0 = 0x2a54a952a54a952aULL;
    uint64_t m3w3 = 0xffffffffffffffffULL, m3w2 = 0xffffff6db6db6db6ULL, m3w1 = 0xdb6db6db6d5ab56aULL, m3w0 = 0xd5ab56ad5ab56ad5ULL;
    uint64_t m4w3 = 0x0000001249249249ULL, m4w2 = 0x7fffffffffffffffULL, m4w1 = 0xffffff5555555555ULL, m4w0 = 0x5555555555555555ULL;
    uint64_t m5w3 = 0xffffffedb6db6db6ULL, m5w2 = 0x8000000000000000ULL, m5w1 = 0x000000aaaaaaaaaaULL, m5w0 = 0xaaaaaaaaaaaaaaaaULL;
    uint64_t m6w3 = 0x0007ffffffffffffULL, m6w2 = 0xfffffffff5555555ULL, m6w1 = 0x5555555555555555ULL, m6w0 = 0x5555555555555555ULL;
    uint64_t m7w3 = 0xfff8000000000000ULL, m7w2 = 0x000000000aaaaaaaULL, m7w1 = 0xaaaaaaaaaaaaaaaaULL, m7w0 = 0xaaaaaaaaaaaaaaaaULL;

    x = or256(shl256(pext256(x, m0w3, m0w2, m0w1, m0w0), 91),
              pext256(x, m1w3, m1w2, m1w1, m1w0));

    x = or256(shl256(pext256(x, m2w3, m2w2, m2w1, m2w0), 191),
              pext256(x, m3w3, m3w2, m3w1, m3w0));

    x = or256(shl256(pext256(x, m4w3, m4w2, m4w1, m4w0), 104),
              pext256(x, m5w3, m5w2, m5w1, m5w0));

    x = or256(shl256(pext256(x, m6w3, m6w2, m6w1, m6w0), 91),
              pext256(x, m7w3, m7w2, m7w1, m7w0));

    input.words[0] = (uint64_t)x.hi;
    input.words[1] = (uint64_t)(x.lo >> 64);
    input.words[2] = (uint64_t)x.lo;
}

#endif
