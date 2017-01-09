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

#define VERSION "0.1.0"
#define HELP_TEXT "A tool for decompressing the XP3 archives used by " \
                  "Nekopara.\n\n" \
                  "   -h, --help\t\tDisplay this help page and " \
                  "exit.\n" \
                  "   -v, --version\tDisplay the currently installed " \
                  "version and exit.\n\n"


/* General subroutine for parsing command-line arguments. Returns a
   "configuration" structure containing everything parsed from argv. */
struct configuration parse_args(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s (ARCHIVE PATH) [OPTIONS]\n", argv[0]);
        exit(1);
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
        {NULL, 0, NULL, 0}
    };

    do {
        current = getopt_long(argc, argv, "hv", long_options, &option_index);
        switch (current) {
            case 'h':
                printf("Usage: %s [OPTIONS] (ARCHIVE PATH)\n\n", argv[0]);
                printf("%s\n", HELP_TEXT);
                exit(EXIT_SUCCESS);
            case 'v':
                printf("Nekopack, version %s\nProgrammed by "
                       "Jakob. <http://tsar-fox.com/>\n", VERSION);
                exit(EXIT_SUCCESS);
        }
        count++;
    } while (current >= 0);

    /* getopt "sorts" the argument array such that all of the flags come
       first. argv[count] is the first positional argument encountered. */
    if (parsed.archive_path == NULL) {
        if (argv[count] == NULL) {
            fprintf(stderr, "No archive path given.\n");
            exit(EXIT_FAILURE);
        }
        parsed.archive_path = argv[count];
    }

    return parsed;
}
