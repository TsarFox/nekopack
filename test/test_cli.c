/* test_cli.c -- MinUnit test cases for cli.c

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

#include "cli.h"
#include "crypto.h"

#include "minunit.h"

extern int tests_run;


const char *test_out_path(void) {
    char *argv[]    = {"nekopack", "-o", "/tmp", "a.xp3"};
    struct params p = parse_args(4, argv);
    mu_assert("[cli] test_out_path: Trailing slash not appended to path",
              !strcmp(p.out, "/tmp/"));
    free(p.out);
    return NULL;
}


const char *test_game_id(void) {
    char *argv[]    = {"nekopack", "-g", "nekopara_volume_1", "a.xp3"};
    struct params p = parse_args(4, argv);
    mu_assert("[cli] test_game_id: Incorrect game ID",
              p.game == NEKOPARA_VOLUME_1);
    free(p.out);
    return NULL;
}


const char *test_vararg_index(void) {
    char *argv[]    = {"nekopack", "-l", "a.xp3"};
    struct params p = parse_args(3, argv);
    mu_assert("[cli] test_vararg_index: Invalid vararg index",
              !strcmp("a.xp3", argv[p.vararg_index]));
    free(p.out);
    return NULL;
}
