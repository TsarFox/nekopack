/* This file is part of Nekopack.

   Copyright (C) 2017 Jakob Tsar-Fox, All Rights Reserved.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include "cli.h"
#include "defs.h"

#define VERSION "1.0.0.rc1"
#define HELP_TEXT "A tool for decompressing the XP3 archives used by " \
                  "Nekopara.\n\n" \
                  "   -h, --help\t\tDisplay this help page and " \
                  "exit.\n" \
                  "   -v, --version\tDisplay the currently installed " \
                  "version and exit.\n\n" \
                  "   -e, --extract\tExtract the contents of the archive. " \
                  "This is the default.\n" \
                  "   -l, --list\t\tList the contents of the archive.\n\n" \
                  "   -a, --archive\tAlternate way of specifying archive" \
                  "to extract.\n" \
                  "   -g, --game\t\tGame the archive is from. Required for " \
                  "file decryption\n" \
                  "   -q, --quiet\t\tDon't display information about " \
                  "extracted files."
#define GAME_CONSTANTS "none, nekopara_volume_0, nekopara_volume_0_steam, " \
                       "nekopara_volume_1, nekopara_volume_1_steam"


/* General subroutine for parsing command-line arguments. Returns a
   "configuration" structure containing everything parsed from argv. */
struct configuration parse_args(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [OPTIONS] (ARCHIVE PATH)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* The struct has to be zeroed out, otherwise argument parsing
       becomes irrelevant and the program will not act as expected. */
    struct configuration parsed;
    memset(&parsed, 0, sizeof(parsed));

    /* count represents the number of options parsed, regardless of
       whether or not they were "long." This is kept track of so that
       the user can supply an archive path as a positional argument. */
    int current = 0, option_index = 0, count = 0;
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"quiet", no_argument, NULL, 'q'},
        {"extract", no_argument, NULL, 'e'},
        {"list", no_argument, NULL, 'l'},
        {"archive", required_argument, NULL, 'a'},
        {"game", required_argument, NULL, 'g'},
        {NULL, 0, NULL, 0}
    };

    do {
        current = getopt_long(argc, argv, "hvelqa:g:", long_options,
                              &option_index);
        switch (current) {
            case 'h':
                printf("Usage: %s [OPTIONS] (ARCHIVE PATH)\n\n", argv[0]);
                printf("%s\n\n", HELP_TEXT);
                printf("Supported Game Crypto:\n%s\n", GAME_CONSTANTS);
                exit(EXIT_SUCCESS);
            case 'v':
                printf("Nekopack, version %s\nProgrammed by "
                       "Jakob. <http://tsar-fox.com/>\n", VERSION);
                exit(EXIT_SUCCESS);
            case 'q':
                parsed.quiet = 1;
                break;
            case 'a':
                count++;
                parsed.archive = optarg;
                break;
            case 'g':
                count++;
                /* Out of concern for efficiency, string comparisons are
                   done here and the source game is stored in an enum. */
                if (!strcmp(optarg, "nekopara_volume_0"))
                    parsed.game = NEKOPARA_VOLUME_0;
                else if (!strcmp(optarg, "nekopara_volume_0_steam"))
                    parsed.game = NEKOPARA_VOLUME_0_STEAM;
                else if (!strcmp(optarg, "nekopara_volume_1"))
                    parsed.game = NEKOPARA_VOLUME_1;
                else if (!strcmp(optarg, "nekopara_volume_1_steam"))
                    parsed.game = NEKOPARA_VOLUME_1_STEAM;
                else
                    parsed.game = NO_CRYPTO;
                break;
            case 'e':
                /* Despite being the default, '-e' is
                   still parsed to provide consisency. */
                parsed.mode = EXTRACT;
                break;
            case 'l':
                parsed.mode = LIST;
        }
        count++;
    } while (current >= 0);

    /* getopt "sorts" the argument array such that all of the flags come
       first. argv[count] is the first positional argument encountered. */
    if (parsed.archive == NULL) {
        if (argv[count] == NULL) {
            fprintf(stderr, "No archive path provided.\n");
            exit(EXIT_FAILURE);
        }
        parsed.archive = argv[count];
    }

    return parsed;
}
