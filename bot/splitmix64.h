#ifndef GONZO_SPLITMIX64_H
#define GONZO_SPLITMIX64_H

#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

typedef struct
{
    uint64_t words[4];
} init_seed;

extern uint64_t splitmix64_state;

static inline uint64_t splitmix64_next()
{
    uint64_t z = (splitmix64_state += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

static inline uint64_t bit_length(uint64_t n)
{
    uint64_t bits = 0;
    while (n)
    {
        bits++;
        n >>= 1;
    }
    return bits;
}

static inline uint64_t fill_to_left(uint64_t el, uint64_t size)
{
    if (size == 0)
    {
        return el;
    }
    while (size < 64)
    {
        el |= el << size;
        size *= 2;
    }
    return el;
}

static inline uint64_t upow(uint64_t base, uint64_t exp)
{
    uint64_t result = 1;
    while (exp > 0)
    {
        if (exp & 1)
            result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

static inline void splitmix64_init(init_seed *seed)
{
    const uint64_t meaning_of_life = 42;

    uint64_t pid = (uint32_t)getpid();
    uint64_t pid_len = bit_length(pid);
    uint64_t pid_res = fill_to_left(pid, pid_len);
    uint64_t sexy_prime_quintuple[] = {5, 11, 17, 23, 29};
    for (uint64_t i = 0; i <= 4; i++)
    {
        uint64_t p = upow(sexy_prime_quintuple[i], pid_res);
        uint64_t p_len = bit_length(p);
        sexy_prime_quintuple[i] = fill_to_left(p, p_len);
    }
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t t = (uint32_t)ts.tv_nsec;
    uint64_t t_len = bit_length(t);
    uint64_t t_res = fill_to_left(t, t_len);

    uint64_t addr = (uintptr_t)&meaning_of_life;
    uint64_t addr_len = bit_length(addr);
    uint64_t addr_res = fill_to_left(addr, addr_len);

    uint64_t entropy = meaning_of_life ^ pid_res ^ t_res ^ addr_res;
    for (uint64_t i = 0; i <= 4; i++)
    {
        entropy ^= sexy_prime_quintuple[i];
    }

    splitmix64_state = entropy;
    for (uint64_t i = 0; i <= 3; i++)
    {
        seed->words[i] = splitmix64_next();
    }
}

#endif