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

#include <inttypes.h> // Needed for debugging at this point.
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <iconv.h>
#include <zlib.h>

#include "cli.h"
#include "defs.h"
#include "decompress.h"
#include "extract.h"
#include "file.h"
#include "write.h"

#define ELIF_MAGIC 0x46696c65
#define FILE_MAGIC 0x656c6946
#define HNFN_MAGIC 0x6e666e68
#define NEKO_MAGIC 0x6f6b656e

elif_node *read_elif_entry(memory_stream *data_stream);
void read_stream(void *destination, Bytef **source, size_t size);


/* Handles decompression of the archive, as well as
   parsing, decrypting and writing the table entries. */
void extract(FILE *archive, uint64_t table_offset) {
    /* eliF and File entries are stored in a linked list as they're
       seen, because the order of entries in XP3 archives is not
       guaranteed to be chronological. calloc is used to prevent the
       "next" pointer from being garbage and causing a segfault. */
    elif_node *elif_root = calloc(sizeof(elif_node), 1);
    elif_node *elif_new;
    file_node *file_root = calloc(sizeof(file_node), 1);
    file_node *file_new;

    uint8_t compressed;
    fseek(archive, table_offset, SEEK_SET);
    fread(&compressed, sizeof(uint8_t), 1, archive);

    int stream_ended = 0;
    memory_stream data_stream;
    if (compressed) {
        data_stream = decompress_file(archive, ftell(archive));
    } else {
        uint64_t decompressed_size;
        fread(&decompressed_size, sizeof(uint64_t), 1, archive);
        /* The second size is irrelevant and therefore ignored. */
        fseek(archive, sizeof(uint64_t), SEEK_CUR);
        data_stream.stream_length = decompressed_size;
        data_stream.data = malloc(decompressed_size);
        data_stream.start = data_stream.data;
        fread(data_stream.data, decompressed_size, 1, archive);
        fclose(archive);
    }

    uint32_t entry_magic;
    uint64_t entry_size;
    do {
        read_stream(&entry_magic, &data_stream.data, sizeof(uint32_t));
        read_stream(&entry_size, &data_stream.data, sizeof(uint64_t));
        printf("entry at 0x%lx\n", data_stream.data - data_stream.start - 12);
        printf("Magic: %" PRIx32 " Size: %" PRIx64 "\n", entry_magic, entry_size);

        switch (entry_magic) {
            /* hnfn, neko, and eliF entries are all identical.

               The size given by the entry header doesn't match the
               actual entry size, so it isn't passed to the function. */
            case ELIF_MAGIC:
            case HNFN_MAGIC:
            case NEKO_MAGIC:
                elif_new = read_elif_entry(&data_stream);
                defer_elif_node(elif_new, elif_root);
                break;
            case FILE_MAGIC:
                file_new = read_file_entry(&data_stream,
                                           data_stream.data + entry_size);
                defer_file_node(file_new, file_root);
                break;
            default:
                printf("End of archive reached.\n");
                stream_ended = 1;
        }
    } while (!stream_ended);

    write_files(file_root, elif_root, data_stream.start);
    test_elif_linked_list(elif_root);
    test_file_linked_list(file_root);
    free_elif_nodes(elif_root);
    free_file_nodes(file_root);
    free(data_stream.start);
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
        /* Strings are terminated by null bytes, but
           they aren't counted in the name size. */
        char *input_buffer = malloc(name_size * 2 + 2);
        file_name = malloc(name_size + 1);
        read_stream(input_buffer, &data_stream->data, name_size * 2 + 2);

        /* iconv is the less-portable glibc way of doing it. It seems to
           be in the OpenBSD manpages, though, so I'm not worried. */
        char *in_start = input_buffer, *out_start = file_name;
        size_t in_size = name_size * 2 + 2, out_size = name_size + 1;
        iconv_t conversion = iconv_open("UTF-8", "UTF-16LE");
        iconv(conversion, &in_start, &in_size, &out_start, &out_size);
        iconv_close(conversion);

        free(input_buffer);
    } else {
        /* It's pretty safe to assume anything
           larger is the copyright notice. */
        data_stream->data += name_size * 2 + 2;
        file_name = strdup("COPYING.txt");
    }

    elif_node *current = malloc(sizeof(elif_node));
    current->key = file_key;
    current->file_name = file_name;
    current->next = NULL;
    return current;
}
