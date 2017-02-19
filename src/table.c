/* table.c -- Code for handling the archive's table section.

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
#include <stdint.h>
#include <stdlib.h>

#include "io.h"
#include "table.h"

#define ELIF_MAGIC 0x46696c65
#define FILE_MAGIC 0x656c6946
#define HNFN_MAGIC 0x6e666e68
#define NEKO_MAGIC 0x6f6b656e


/* Returns the root of a linked list containing all of the files listed
   in the archive's table section. */
struct table_entry *parse_table(struct stream *s) {
    struct table_entry *cur, *root = calloc(sizeof(struct table_entry), 1);

    bool     ended = false;
    uint32_t magic;
    uint64_t size;

    do {
        stream_read(&magic, s, sizeof(uint32_t));
        stream_read(&size, s, sizeof(uint64_t));

        switch (magic) {
            case ELIF_MAGIC:
            case HNFN_MAGIC:
            case NEKO_MAGIC:
                break;
            case FILE_MAGIC:
                break;
            default:
                ended = 1;
        }
    } while (!ended);
}


/* Inserts `e` to the end of the linked list specified by `root`. */
void entry_append(struct table_entry *root, struct table_entry *e) {
    struct table_entry *cur;
    for (cur = root; cur->next != NULL; cur = cur->next);
    cur->next = e;
}


/* Frees every entry in the linked list specified by `cur`. */
void entry_free(struct table_entry *cur) {
    if (cur->next != NULL)
        entry_free(cur->next);
    free(cur);
}
