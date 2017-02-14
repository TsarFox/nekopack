/* header.c -- Code for handling the archive's header section.

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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "io.h"

static bool is_xp3(struct xp3_header *h);
static bool is_supported(struct xp3_header *h);


/* Reads data from the given stream into a newly allocated xp3_header
   structure. The stream is assumed to be at the header's beginning.
   NULL is returned if the header contains an invalid magic number, or
   if the archive's version is not supported. */
struct xp3_header *read_header(struct stream *s) {
    struct xp3_header *h = malloc(sizeof(struct xp3_header));
    if (h == NULL) return NULL;
    stream_read(h, s, sizeof(struct xp3_header));
    if (!is_xp3(h) || !is_supported(h)) {free(h); return NULL;}
    return h;
}


/* Checks that the header contains the correct magic number. */
static bool is_xp3(struct xp3_header *h) {
    return !memcmp(h->magic, XP3_MAGIC, 11);
}


/* Checks that the archive is of XP3 version 2. */
static bool is_supported(struct xp3_header *h) {
    return h->version == 1;
}
