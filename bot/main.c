#define _POSIX_C_SOURCE 200809L

#define GONZO_VERSION_MAJOR 0
#define GONZO_VERSION_MINOR 1
#define MAX_SIZE 13

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

#include "field.h"
#include "history.h"

#include "splitmix64.h"
#include "xoshiro.h"
#include "zobrist.h"

// global settings
uint8_t SIZE;
uint8_t KOMI;

// zobrist hashing
uint64_t splitmix64_state;
uint64_t zobrist_table[MAX_SIZE][MAX_SIZE][2]; // x y color

// history main line
history_t main_line;

int main(void)
{
    // set type to line buffer
    setvbuf(stdout, NULL, _IOLBF, 0);

    // initialize seeding & zobrist
    init_seed seed;
    splitmix64_init(&seed);
    zobrist_init(&seed);

    // initialize main line
    history_init(&main_line);

    // main loop
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
        else if (strcmp(command_buf, "clear_board") == 0)
        {
            history_clear(&main_line);
            printf("= \n\n");
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
                continue; // not really a reason to quit
            }

            // this is probably faster (only a compare score w == score b => w wins)
            KOMI = (uint8_t)fkomi;
            printf("= \n\n");
            continue;
        }
        else if (strncmp(command_buf, "play", 4) == 0)
        {
            char color = tolower(arg1[0]);
            if (color != 'b' && color != 'w')
            {
                printf("? invalid color\n\n");
                continue;
            }

            char x_char = tolower(arg2[0]);
            int x = x_char - 'a';
            if (x < 0)
            {
                printf("? invalid column (x)");
                continue;
            }

            int y = atoi(arg2 + 1) - 1;
            printf("? x %d y %d\n\n", x, y);
            break;
        }
        else if (strcmp(command_buf, "quit") == 0)
        {
            break;
        }
        else
        {
            printf("? command not found\n\n");
        }
    }

    return 0;
}
