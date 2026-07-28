#ifndef GONZO_PERMUTATIONS_H
#define GONZO_PERMUTATIONS_H

#include <stdint.h>
#include <math.h>
#include <immintrin.h>

typedef unsigned __int128 uint128_t;

#define U128(hi, lo) (((uint128_t)(hi##ULL) << 64) | (lo##ULL))

static inline uint128_t rol(uint128_t x, unsigned rot)
{
    rot &= 127;
    return (x << rot) | (x >> ((128 - rot) & 127));
}

static inline uint128_t bswap(uint128_t x)
{
    uint64_t hi = (uint64_t)(x >> 64);
    uint64_t lo = (uint64_t)x;
    return ((uint128_t)__builtin_bswap64(lo) << 64) | __builtin_bswap64(hi);
}

static inline uint128_t pdep128(uint128_t src, uint64_t mask_hi, uint64_t mask_lo)
{
    unsigned count_lo = __builtin_popcountll(mask_lo);
    uint64_t lo = _pdep_u64((uint64_t)src, mask_lo);
    uint64_t hi = _pdep_u64((uint64_t)(src >> count_lo), mask_hi);
    return ((uint128_t)hi << 64) | lo;
}

static inline uint64_t pext128(uint128_t x, uint64_t mask_hi, uint64_t mask_lo)
{
    unsigned count_lo = __builtin_popcountll(mask_lo);
    uint64_t lo = _pext_u64((uint64_t)x, mask_lo);
    uint64_t hi = _pext_u64((uint64_t)(x >> 64), mask_hi);
    return lo | (hi << count_lo);
}

// im sorry
typedef struct
{
    uint128_t hi;
    uint128_t lo;
} uint256_t;

static inline uint256_t bswap256(uint256_t x)
{
    uint256_t r;
    r.hi = bswap(x.lo);
    r.lo = bswap(x.hi);
    return r;
}

// rot must be in [0, 255]
static inline uint256_t rol256(uint256_t x, unsigned rot)
{
    rot &= 255;
    if (rot == 0)
        return x;
    uint256_t r;
    if (rot < 128)
    {
        r.hi = (x.hi << rot) | (x.lo >> (128 - rot));
        r.lo = (x.lo << rot) | (x.hi >> (128 - rot));
    }
    else if (rot == 128)
    {
        r.hi = x.lo;
        r.lo = x.hi;
    }
    else
    {
        unsigned s = rot - 128;
        r.hi = (x.lo << s) | (x.hi >> (128 - s));
        r.lo = (x.hi << s) | (x.lo >> (128 - s));
    }
    return r;
}

// k may be any value in [0, 255]
static inline uint256_t shr256(uint256_t x, unsigned k)
{
    k &= 255;
    uint256_t r;
    if (k == 0)
        r = x;
    else if (k < 128)
    {
        r.hi = x.hi >> k;
        r.lo = (x.lo >> k) | (x.hi << (128 - k));
    }
    else if (k == 128)
    {
        r.hi = 0;
        r.lo = x.hi;
    }
    else
    {
        r.hi = 0;
        r.lo = x.hi >> (k - 128);
    }
    return r;
}

// k in [0, 255]
static inline uint256_t shl256(uint256_t x, unsigned k)
{
    k &= 255;
    uint256_t r;
    if (k == 0)
        r = x;
    else if (k < 128)
    {
        r.lo = x.lo << k;
        r.hi = (x.hi << k) | (x.lo >> (128 - k));
    }
    else if (k == 128)
    {
        r.lo = 0;
        r.hi = x.lo;
    }
    else
    {
        r.lo = 0;
        r.hi = x.lo << (k - 128);
    }
    return r;
}

static inline uint256_t and256(uint256_t a, uint256_t b)
{
    uint256_t r = {a.hi & b.hi, a.lo & b.lo};
    return r;
}

static inline uint256_t or256(uint256_t a, uint256_t b)
{
    uint256_t r = {a.hi | b.hi, a.lo | b.lo};
    return r;
}

static inline uint256_t from_u64(uint64_t v)
{
    uint256_t r = {0, (uint128_t)v};
    return r;
}

// generalizes pdep128 to 4 words
static inline uint256_t pdep256(uint256_t src, uint64_t mask3, uint64_t mask2, uint64_t mask1, uint64_t mask0)
{
    unsigned c0 = __builtin_popcountll(mask0);
    unsigned c1 = __builtin_popcountll(mask1);
    unsigned c2 = __builtin_popcountll(mask2);

    uint64_t w0 = _pdep_u64((uint64_t)src.lo, mask0);
    uint64_t w1 = _pdep_u64((uint64_t)shr256(src, c0).lo, mask1);
    uint64_t w2 = _pdep_u64((uint64_t)shr256(src, c0 + c1).lo, mask2);
    uint64_t w3 = _pdep_u64((uint64_t)shr256(src, c0 + c1 + c2).lo, mask3);

    uint256_t r;
    r.lo = ((uint128_t)w1 << 64) | w0;
    r.hi = ((uint128_t)w3 << 64) | w2;
    return r;
}

// .. pext128 to 4 words
static inline uint256_t pext256(uint256_t x, uint64_t mask3, uint64_t mask2, uint64_t mask1, uint64_t mask0)
{
    unsigned c0 = __builtin_popcountll(mask0);
    unsigned c1 = __builtin_popcountll(mask1);
    unsigned c2 = __builtin_popcountll(mask2);

    uint64_t e0 = _pext_u64((uint64_t)x.lo, mask0);
    uint64_t e1 = _pext_u64((uint64_t)(x.lo >> 64), mask1);
    uint64_t e2 = _pext_u64((uint64_t)x.hi, mask2);
    uint64_t e3 = _pext_u64((uint64_t)(x.hi >> 64), mask3);

    uint256_t r = from_u64(e0);
    r = or256(r, shl256(from_u64(e1), c0));
    r = or256(r, shl256(from_u64(e2), c0 + c1));
    r = or256(r, shl256(from_u64(e3), c0 + c1 + c2));
    return r;
}

#endif
