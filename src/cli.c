/* cli.c -- Code for providing a command-line interface.

   Copyright (C) 2017 Jakob Kreuze, All Rights Reserved.

   This file is part of Nekopack.

   Nekopack is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation, either version 3 of the License, or (at
   your option) any later version.

   Nekopack is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Nekopack. If not, see <http://www.gnu.org/licenses/>. */

#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include "cli.h"
#include "crypto.h"


/* Copies `optarg` into the out field of `p` and ensures that it ends in
   a trailing path delimiter. */
static void parse_output_path(const char *optarg, struct params *p) {
    p->out_len = strlen(optarg);
    p->out     = malloc(p->out_len + 2);
    strcpy(p->out, optarg);
    if (p->out[p->out_len - 1] != '/') {
        p->out[p->out_len] = '/';
        p->out_len        += 1;
    }
}


/* Updates the game field of `p` to properly reflect the game ID
   specified by the string, `optarg`. */
static void parse_game_id(const char *optarg, struct params *p) {
    if (!strcmp(optarg, "nekopara_volume_0"))
        p->game = NEKOPARA_VOLUME_0;
    else if (!strcmp(optarg, "nekopara_volume_0_steam"))
        p->game = NEKOPARA_VOLUME_0_STEAM;
    else if (!strcmp(optarg, "nekopara_volume_1"))
        p->game = NEKOPARA_VOLUME_1;
    else if (!strcmp(optarg, "nekopara_volume_1_steam"))
        p->game = NEKOPARA_VOLUME_1_STEAM;
}


/* Returns a params structure parsed from `argv`. */
struct params parse_args(int argc, char **argv) {
    struct params p = {0};

    if (argc < 2) {
        p.mode = USAGE;
        return p;
    }
    p.mode = EXTRACT;

    int cur = 0, opt_index = 0, count = 0;
    static struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'V'},
        {"verbose", no_argument, NULL, 'v'},
        {"extract", no_argument, NULL, 'e'},
        {"list", no_argument, NULL, 'l'},
        {"output", no_argument, NULL, 'o'},
        {"game", no_argument, NULL, 'g'},
        {NULL, 0, NULL, 0}
    };

    do {
        count++;
        cur = getopt_long(argc, argv, "hVvelqo:g:", long_opts, &opt_index);
        switch (cur) {
            case 'h':
                p.mode = HELP;
                return p;
            case 'V':
                p.mode = VERSION;
                return p;
            case 'v':
                p.verbose = true;
                break;
            case 'o':
                count++;
                parse_output_path(optarg, &p);
                break;
            case 'g':
                count++;
                parse_game_id(optarg, &p);
            case 'e':
                p.mode = EXTRACT;
                break;
            case 'l':
                p.mode = LIST;
        }
    } while (cur >= 0);

    if (count == argc) {
        p.mode = USAGE;
        return p;
    }
    p.vararg_index = count;

    if (p.out == NULL) {
        p.out_len = 2;
        p.out = malloc(3);
        strcpy(p.out, "./");
    }

    return p;
}


/* Frees allocated memory associated with `p`. */
void params_free(struct params p) {
    free(p.out);
}
