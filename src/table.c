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
#include <string.h>

#include "encoding.h"
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
                read_elif(s, root);
            case FILE_MAGIC:
                break;
            default:
                ended = 1;
        }
    } while (!ended);
}


/* Reads the contents of an eliF chunk. If there is an entry with a
   matching key in the linked list specified by `root`, that structure
   will be modified. Otherwise, a new entry will be created and appended
   to the linked list. */
void read_elif(struct stream *s, struct table_entry *root) {
    char               *name, *buf;
    uint16_t           name_size;
    uint32_t           key;
    struct table_entry *cur;

    stream_read(&key, s, sizeof(uint32_t));
    stream_read(&name_size, s, sizeof(uint16_t));

    /* The value provided by the archive represents the number of
       UTF-16LE characters, not the number of bytes in the string. */
    name_size = name_size * 2 + 2;

    for (cur = root; cur != NULL && cur->key != key; cur = cur->next);
    if (cur == NULL) {
        cur = calloc(sizeof(struct table_entry), 1);
        if (cur == NULL) return;
        entry_append(root, cur);
    }

    if (name_size < 0x100) {
        buf = malloc(name_size);
        name = malloc(name_size);
        if (buf == NULL || name == NULL) return;

        stream_read(buf, s, name_size);
        utf16le_decode(buf, name, name_size);
        free(buf);

        name = realloc(name, strlen(name) + 1);
        if (name == NULL) return;
    } else {
        /* strdup isn't defined in ISO/IEC 9899:1999 C. */
        name = malloc(14);
        if (name == NULL) return;
        strncpy(name, "COPYRIGHT.txt", 14);
    }
    cur->filename = name;
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
