/* header.c -- Code for handling the archive's header section.

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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "io.h"

static bool is_xp3(struct header *h);
static bool is_supported(struct header *h);


/* Reads from the given stream into a newly allocated header structure.
   NULL is returned if the header contains an invalid magic number, or
   if the archive's version is not supported. */
struct header *read_header(struct stream *s) {
    struct header *h = malloc(sizeof(struct header));
    if (h == NULL) return NULL;

    /* The header structure can't be read into directly because of
       potential alignment issues. */
    stream_read(h->magic, s, 11);
    stream_read(&h->info_offset, s, sizeof(uint64_t));
    stream_read(&h->version, s, sizeof(uint32_t));
    stream_read(&h->flags, s, sizeof(uint8_t));
    stream_read(&h->table_size, s, sizeof(uint64_t));
    stream_read(&h->table_offset, s, sizeof(uint64_t));

    if (!is_xp3(h) || !is_supported(h)) {
        free(h);
        return NULL;
    }
    return h;
}


/* Checks that the header contains the correct magic number. */
static bool is_xp3(struct header *h) {
    return !memcmp(h->magic, XP3_MAGIC, 11);
}


/* Checks that the archive's version is supported, and that it is marked
   as compatible with the KiriKiriZ engine. */
static bool is_supported(struct header *h) {
    return h->version == 1 && h->flags & 0x80;
}
