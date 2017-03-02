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

#define ADLR_MAGIC 0x726c6461
#define SEGM_MAGIC 0x6d676573
#define INFO_MAGIC 0x6f666e69
#define TIME_MAGIC 0x656d6974


/* Reads a segm chunk into the table_entry specified by `tmp`. */
static void read_segm(struct stream *s, struct table_entry *tmp, uint64_t count) {
    tmp->segment_count = count;
    tmp->segments = malloc(sizeof(struct segment *) * count);
    if (tmp->segments == NULL) return;

    for (uint64_t i = 0; i < count; i++) {
        tmp->segments[i] = malloc(sizeof(struct segment));
        if (tmp->segments[i] == NULL) return;
        stream_read(&tmp->segments[i]->compressed, s, sizeof(uint32_t));
        stream_read(&tmp->segments[i]->offset, s, sizeof(uint64_t));
        stream_read(&tmp->segments[i]->decompressed_size, s, sizeof(uint64_t));
        stream_read(&tmp->segments[i]->compressed_size, s, sizeof(uint64_t));
    }
}


/* Reads an adlr chunk into the table_entry specified by `tmp`. */
static void read_adlr(struct stream *s, struct table_entry *tmp) {
    stream_read(&tmp->key, s, sizeof(uint32_t));
}


/* Reads a time chunk into the table_entry specified by `tmp`. */
static void read_time(struct stream *s, struct table_entry *tmp) {
    stream_read(&tmp->ctime, s, sizeof(uint64_t));
}


/* Returns the root of a linked list containing all of the files listed
   in the archive's table section. */
struct table_entry *read_table(struct stream *s) {
    struct table_entry *root = calloc(sizeof(struct table_entry), 1);

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
                break;
            case FILE_MAGIC:
                read_file(s, root);
                break;
            default:
                ended = 1;
        }
    } while (!ended);

    return root;
}


/* Reads the contents of a File chunk. If there is an entry with a
   matching key in the linked list specified by `root`, that structure
   will be modified. Otherwise, a new entry will be created and appended
   to the linked list. */
void read_file(struct stream *s, struct table_entry *root) {
    bool               ended = false;
    uint32_t           magic;
    uint64_t           size;
    struct table_entry *cur, *tmp = calloc(sizeof(struct table_entry), 1);

    do {
        stream_read(&magic, s, sizeof(uint32_t));
        stream_read(&size, s, sizeof(uint64_t));

        switch (magic) {
            case ADLR_MAGIC:
                read_adlr(s, tmp);
                break;
            case SEGM_MAGIC:
                read_segm(s, tmp, size / 28);
                break;
            case INFO_MAGIC:
                stream_seek(s, size, SEEK_CUR);
                break;
            case TIME_MAGIC:
                read_time(s, tmp);
                break;
            default:
                ended = true;
                stream_seek(s, -sizeof(uint32_t) - sizeof(uint64_t), SEEK_CUR);
        }
    } while (!ended);

    cur = get_node(root, tmp->key);
    cur->segment_count = tmp->segment_count;
    cur->segments = tmp->segments;
    cur->ctime = tmp->ctime;
    free(tmp);
}


/* Reads the contents of an eliF chunk. If there is an entry with a
   matching key in the linked list specified by `root`, that structure
   will be modified. Otherwise, a new entry will be created and appended
   to the linked list. */
void read_elif(struct stream *s, struct table_entry *root) {
    char               *name, *buf, *tmp;
    uint16_t           name_len;
    uint32_t           key;
    struct table_entry *cur;

    stream_read(&key, s, sizeof(uint32_t));
    stream_read(&name_len, s, sizeof(uint16_t));

    /* The value provided by the archive represents the number of
       UTF-16LE characters, not the number of bytes in the string. */
    name_len = name_len * 2 + 2;

    cur = get_node(root, key);
    if (cur->filename != NULL) {
        return;
    }

    if (name_len < 0x100) {
        buf = malloc(name_len);
        name = malloc(name_len);
        if (buf == NULL || name == NULL) return;

        stream_read(buf, s, name_len);
        utf16le_decode(buf, name, name_len);
        free(buf);

        tmp = realloc(name, strlen(name) + 1);
        if (tmp == NULL) {
            free(name);
            return;
        }
        name = tmp;
    } else {
        /* strdup isn't defined in ISO/IEC 9899:1999 C. */
        name = malloc(14);
        if (name == NULL) return;
        strncpy(name, "COPYRIGHT.txt", 14);
        stream_seek(s, name_len, SEEK_CUR);
    }
    cur->filename = name;
}


/* Traverses `root` for a node with the given key. If the linked list
   lacks a node with the key, a new node is created and appended. */
struct table_entry *get_node(struct table_entry *root, uint32_t key) {
    struct table_entry *cur;
    for (cur = root; cur != NULL && cur->key != key; cur = cur->next);
    if (cur == NULL) {
        cur = calloc(sizeof(struct table_entry), 1);
        if (cur == NULL) return NULL;
        entry_append(root, cur);
        cur->key = key;
    }
    return cur;
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
    if (cur->filename != NULL)
        free(cur->filename);
    if (cur->segments != NULL) {
        for (uint64_t i = 0; i < cur->segment_count; i++)
            free(cur->segments[i]);
        free(cur->segments);
    }
    free(cur);
}
