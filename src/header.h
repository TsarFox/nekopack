/* header.h -- Code for handling the archive's header section.

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

#pragma once

#include <stdint.h>

#include "io.h"

#define XP3_MAGIC "XP3\x0d\x0a\x20\x0a\x1a\x8b\x67\x01"

/* Structure representing the header section of an XP3 archive. */
struct header {
    char     magic[11];         /* Identifier for the archive. */
    uint64_t info_offset;       /* Offset to `table_size`. */
    uint32_t version;           /* Raw value containing the archive version. */
    uint64_t table_size;        /* The size of the table section. (?) */
    uint8_t  flags;             /* A flags variable for the archive. (?) */
    uint64_t table_offset;      /* Offset to the archive table. */
};

/* Reads data from the given stream into a newly allocated header
   structure. The stream is assumed to be at the header's beginning.
   NULL is returned if the header contains an invalid magic number, or
   if the archive's version is not supported. */
struct header *read_header(struct stream *s);
