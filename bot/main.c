#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

#include "field.h"

#include "splitmix64.h"
#include "xoshiro.h"

#define GONZO_VERSION_MAJOR 0
#define GONZO_VERSION_MINOR 1
#define MAX_SIZE 13

uint64_t splitmix64_state;

uint64_t zobrist_table[MAX_SIZE][MAX_SIZE][2];

uint8_t SIZE;
uint8_t KOMI;

int main(void)
{
    init_seed seed;
    splitmix64_init(&seed);

    for (uint8_t x = 0; x < MAX_SIZE; x++)
    {
        for (uint8_t y = 0; y < MAX_SIZE; y++)
        {
            for (uint8_t c = 0; c <= 1; c++)
            {
                zobrist_table[x][y][c] = xoshiro256ss_next(&seed);
            }
        }
    }

    while (1)
    {
        char line[128];
        if (!fgets(line, sizeof(line), stdin))
        {
            break;
        }

        char command_buf[128] = "", arg1[5] = "", arg2[4] = "";
        sscanf(line, "%127s %4s %3s", command_buf, arg1, arg2);

        if (command_buf[0] == '\0')
        {
            continue;
        }

        if (strcmp(command_buf, "name") == 0)
        {
            printf("= Gonzo the Great\n\n");
            continue;
        }
        else if (strcmp(command_buf, "version") == 0)
        {
            printf("= %d.%d\n\n", GONZO_VERSION_MAJOR, GONZO_VERSION_MINOR);
            continue;
        }
        else if (strncmp(command_buf, "boardsize", 9) == 0)
        {
            int size = atoi(arg1);
            if (size != 9 && size != 13)
            {
                printf("? invalid size, only 9/13\n\n");
                break;
            }

            SIZE = (uint8_t)size;
            printf("= \n\n");
            continue;
        }
        else if (strncmp(command_buf, "komi", 4) == 0)
        {
            // .5 can be expected
            float fkomi = atof(arg1);
            if (fmod(fkomi, 1.0) != 0.5)
            {
                printf("? komi does not end with .5\n\n");
                break;
            }

            // this is probably faster (only a compare score w == score b => w wins)
            KOMI = (uint8_t)fkomi;
            printf("= \n\n");
            continue;
        }
        else if (strcmp(command_buf, "quit") == 0)
        {
            break;
        }
        else {
            printf("? command not found\n\n");
        }
    }

    return 0;
}
