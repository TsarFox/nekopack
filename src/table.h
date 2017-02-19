/* table.h -- Code for handling the archive's table section.

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
#include <stdint.h>

#include "io.h"


/* Structure representing one segment associated with a File entry. */
struct segment {
    bool     compressed; /* Whether or not the segment is compressed. */
    uint64_t offset; /* Offset to the segment's beginning. */
    uint64_t compressed_size; /* Size of compressed segment. */
    uint64_t decompressed_size; /* Size of decompressed segment. */
};

/* Structure representing an entry in the archive's table section. */
struct table_entry {
    bool               encrypted; /* Whether or not it's encrypted. */
    bool               compressed; /* Whether or not it's compressed. */
    uint32_t           key; /* File-specific key for encryption. */
    uint64_t           ctime; /* Timestamp of creation time. */
    uint64_t           segment_count; /* Number of segments. */
    struct segment     **segments; /* Array of associated segments. */
    struct table_entry *next; /* Pointer to the next entry. */
};


/* Returns the root of a linked list containing all of the files listed
   in the archive's table section. */
struct table_entry *parse_table(struct stream *s);

/* Inserts `e` to the end of the linked list specified by `root`. */
void entry_append(struct table_entry *root, struct table_entry *e);

/* Frees every entry in the linked list specified by `cur`. */
void entry_free(struct table_entry *cur);
