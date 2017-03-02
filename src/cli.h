/* cli.h -- Code for providing a command-line interface.

   Copyright (C) 2017 Jakob Tsar-Fox, All Rights Reserved.

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

#pragma once

#include <stdbool.h>
#include <stddef.h>

/* Identifiers for the mode of operation. */
enum {
    USAGE,
    HELP,
    VERSION,
    LIST,
    EXTRACT,
};

/* Structure for storing options set from the command-line. */
struct params {
    int    mode; /* Current mode of operation. */
    int    game; /* Which encryption keys to use. */
    bool   quiet; /* Whether or not to suppress progress messages. */
    int    vararg_index; /* Start index of paths in argv. */
    char   *out; /* Path to extract files to. */
    size_t out_len; /* Length of the output path string. */
};

/* Returns a params structure parsed from `argv`. */
struct params parse_args(int argc, char **argv);

/* Frees allocated memory associated with `p`. */
void params_free(struct params p);
