/* This file is part of Nekopack.

   Copyright (C) 2017 Jakob Tsar-Fox, All Rights Reserved.

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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <iconv.h>
#include <zlib.h>

#include "cli.h"
#include "defs.h"
#include "extract.h"
#include "file.h"
#include "write.h"

#define ELIF_MAGIC 0x46696c65
#define FILE_MAGIC 0x656c6946
#define HNFN_MAGIC 0x6e666e68
#define NEKO_MAGIC 0x6f6b656e

elif_node *read_elif_entry(memory_stream *data_stream);
void read_stream(void *destination, Bytef **source, size_t size);


/* Decrypts and writes files in the XP3 archive
   to disk according to table entries. */
void extract(memory_stream data_stream, FILE *archive) {
    /* eliF and File entries are stored in a linked list as they're
       seen because the order of entries in XP3 archives is not
       guaranteed to be chronological. calloc is used to prevent the
       "next" pointer from being garbage and causing a segfault. */
    elif_node *elif_new, *elif_root = calloc(sizeof(elif_node), 1);
    file_node *file_new, *file_root = calloc(sizeof(file_node), 1);

    int stream_ended = 0;
    uint32_t entry_magic;
    uint64_t entry_size;
    do {
        read_stream(&entry_magic, &data_stream.data, sizeof(uint32_t));
        read_stream(&entry_size, &data_stream.data, sizeof(uint64_t));

        switch (entry_magic) {
            /* hnfn, neko, and eliF entries are all identical.

               The size given by the entry header doesn't match the
               actual entry size, so it isn't passed to the function. */
            case ELIF_MAGIC:
            case HNFN_MAGIC:
            case NEKO_MAGIC:
                elif_new = read_elif_entry(&data_stream);
                if (elif_new == NULL) {
                    fprintf(stderr, "Insufficient memory.\n");
                    free_elif_nodes(elif_root);
                    free_file_nodes(file_root);
                    return;
                }
                defer_elif_node(elif_new, elif_root);
                break;
            case FILE_MAGIC:
                file_new = read_file_entry(&data_stream,
                                           data_stream.data + entry_size);
                if (file_new == NULL) {
                    fprintf(stderr, "Insufficient memory.\n");
                    free_elif_nodes(elif_root);
                    free_file_nodes(file_root);
                    return;
                }
                defer_file_node(file_new, file_root);
                break;
            default:
                stream_ended = 1;
        }
    } while (!stream_ended);

    write_files(file_root, elif_root, archive);
    free_elif_nodes(elif_root);
    free_file_nodes(file_root);
}


/* Simply lists the contents of an archive, ignoring File entries. */
void list(memory_stream data_stream) {
    int stream_ended = 0;
    uint32_t entry_magic;
    uint64_t entry_size;
    elif_node *temporary;
    do {
        read_stream(&entry_magic, &data_stream.data, sizeof(uint32_t));
        read_stream(&entry_size, &data_stream.data, sizeof(uint64_t));

        switch (entry_magic) {
            case ELIF_MAGIC:
            case HNFN_MAGIC:
            case NEKO_MAGIC:
                temporary = read_elif_entry(&data_stream);
                printf("%s\n", temporary->file_name);
                free(temporary->file_name);
                free(temporary);
                break;
            case FILE_MAGIC:
                data_stream.data += entry_size;
                break;
            default:
                stream_ended = 1;
        }
    } while (!stream_ended);
}


/* Wrapper for memcpy which increments the source operand by
   the amount of bytes read to simulate a file stream. */
void read_stream(void *destination, Bytef **source, size_t size) {
    memcpy(destination, *source, size);
    *source += size;
}


/* Returns a pointer to a elif_node containing the filename
   and key, which can be deferred in a linked list. */
elif_node *read_elif_entry(memory_stream *data_stream) {
    uint32_t file_key;
    uint16_t name_size;
    read_stream(&file_key, &data_stream->data, sizeof(uint32_t));
    read_stream(&name_size, &data_stream->data, sizeof(uint16_t));

    char *file_name;
    if (name_size < 0x100) {
        /* Strings are terminated by null bytes,
           which aren't counted in the name size. */
        char *input_buffer = malloc(name_size * 2 + 2);
        read_stream(input_buffer, &data_stream->data, name_size * 2 + 2);
        if (input_buffer == NULL)
            return NULL;

        /* name_size + 1 is unreliable since we're going back to UTF-8
           and some characters (especially Japanese ones) can be wide.
           To compensate we just allocate a buffer that could fit the
           UTF-16LE data and utilize null-bytes to terminate names. */
        file_name = malloc(name_size * 2 + 2);
        if (file_name == NULL) {
            free(input_buffer);
            return NULL;
        }

        /* iconv is the less-portable glibc way of doing it. It seems to
           be in the OpenBSD manpages, though, so I'm not worried. */
        size_t in_size = name_size * 2 + 2, out_size = name_size * 2 + 2;
        char *in_start = input_buffer, *out_start = file_name;
        iconv_t conversion = iconv_open("UTF-8", "UTF-16LE");
        iconv(conversion, &in_start, &in_size, &out_start, &out_size);
        iconv_close(conversion);

        free(input_buffer);
    } else {
        /* It's safe to assume too large is the copyright notice. */
        data_stream->data += name_size * 2 + 2;
        file_name = strdup("COPYING.txt");
    }

    elif_node *current = malloc(sizeof(elif_node));
    if (current == NULL) {
        free(file_name);
        return NULL;
    }
    current->key = file_key;
    current->file_name = file_name;
    current->next = NULL;
    return current;
}
