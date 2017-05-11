/* table.h -- Code for handling the archive's table section.

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

#include <stdbool.h>
#include <stdint.h>

#include "io.h"


/* Structure representing one segment associated with a File entry. */
struct segment {
    bool     compressed;        /* Whether or not the segment is compressed. */
    uint64_t offset;            /* Offset to the segment's beginning. */
    uint64_t compressed_size;   /* Size of compressed segment. */
    uint64_t decompressed_size; /* Size of decompressed segment. */
};

/* Structure representing an entry in the archive's table section. */
struct table_entry {
    uint32_t           key;           /* File-specific key for encryption. */
    uint64_t           ctime;         /* Timestamp of creation time. */
    uint64_t           segment_count; /* Number of segments. */
    char               *filename;     /* String containing file's name. */
    struct segment     **segments;    /* Array of associated segments. */
    struct table_entry *next;         /* Pointer to the next entry. */
};


/* Returns the root of a linked list containing all of the files listed
   in the archive's table section. */
struct table_entry *read_table(struct stream *s);

/* Reads the contents of an eliF chunk. If there is an entry with a
   matching key in the linked list specified by `root`, that structure
   will be modified. Otherwise, a new entry will be created and appended
   to the linked list. */
void read_elif(struct stream *s, struct table_entry *root);

/* Reads the contents of a File chunk. If there is an entry with a
   matching key in the linked list specified by `root`, that structure
   will be modified. Otherwise, a new entry will be created and appended
   to the linked list. */
void read_file(struct stream *s, struct table_entry *root);

/* Inserts the file specified by `path` into the table linked list
   specified by `root`. */
struct table_entry *add_file(struct table_entry *root, char *path);

/* Traverses `root` for a node with the given key. If the linked list
   lacks a node with the key, a new node is created and appended. */
struct table_entry *get_node(struct table_entry *root, uint32_t key);

/* Inserts `e` to the end of the linked list specified by `root`. */
void entry_append(struct table_entry *root, struct table_entry *e);

/* Frees every entry in the linked list specified by `cur`. */
void entry_free(struct table_entry *cur);

/* Dumps the XP3 table specified by `root` into `fp`. */
void dump_table(FILE *fp, struct table_entry *root);

/* Inserts the file specified by `path` into the table linked list
   specified by `root`. */
struct table_entry *add_file(struct table_entry *root, char *path);
