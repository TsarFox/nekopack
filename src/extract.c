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

#include "defs.h"
#include "decompress.h"
#include "extract.h"
#include "file.h"
#include "write.h"

#define ELIF_MAGIC 0x46696c65
#define FILE_MAGIC 0x656c6946
#define HNFN_MAGIC 0x6e666e68

void read_stream(void *destination, Bytef **source, size_t size);
void read_file_entry(memory_stream *data_stream, Bytef *section_end);
node *read_elif_entry(memory_stream *data_stream);


/* Handles decompression of the archive, as well as
   parsing, decrypting and writing the table entries. */
void extract(FILE *archive, uint64_t table_offset) {
    /* Filenames are stored in a linked list as they're seen.
       calloc is used to prevent the next pointer from being junk. */
    node *root = calloc(sizeof(node), 1);

    uint8_t compressed;
    fseek(archive, table_offset, SEEK_SET);
    fread(&compressed, sizeof(uint8_t), 1, archive);

    int stream_ended = 0;
    memory_stream data_stream;
    if (compressed) {
        data_stream = decompress_file(archive, ftell(archive));
    } else {
        fprintf(stderr, "This archive is not supported.\n");
        exit(EXIT_FAILURE);
    }

    /* The header for every entry in the XP3 archive format contains
       a magic number, followed by the size of the entry. */
    uint32_t entry_magic;
    uint64_t entry_size;
    /* Implement "last filename" variable, which gets freed
       and reallocated every time an elif entry is read. */
    do {
        read_stream(&entry_magic, &data_stream.data, sizeof(uint32_t));
        read_stream(&entry_size, &data_stream.data, sizeof(uint64_t));
        printf("entry at 0x%lx\n", data_stream.data - data_stream.start - 12);
        printf("Magic: %" PRIx32 " Size: %" PRIx64 "\n", entry_magic, entry_size);

        switch (entry_magic) {
            /* hnfn and eliF entries are identical. The size given by
               the entry header doesn't match the actual entry size,
               so it isn't passed to the function. */
            case HNFN_MAGIC:
            case ELIF_MAGIC:
                defer_node(read_elif_entry(&data_stream), root);
                break;
            case FILE_MAGIC:
                read_file_entry(&data_stream, data_stream.data + entry_size);
                break;
            default:
                printf("End of archive reached.\n");
                stream_ended = 1;
        }
    } while (!stream_ended);

    test_linked_list(root);
    free_node(root);

    free(data_stream.start);
}


/* Wrapper for memcpy which increments the source operand by
   the amount of bytes read to simulate a file stream. */
void read_stream(void *destination, Bytef **source, size_t size) {
    memcpy(destination, *source, size);
    *source += size;
}


/* Document */
node *read_elif_entry(memory_stream *data_stream) {
    uint16_t name_size;
    uint32_t name_hash;
    read_stream(&name_hash, &data_stream->data, sizeof(uint32_t));
    read_stream(&name_size, &data_stream->data, sizeof(uint16_t));

    /* Strings are terminated by null bytes, 
       which aren't counted in the name size. */
    char *input_buffer = malloc(name_size * 2 + 2);
    char *file_name = malloc(name_size + 1);
    read_stream(input_buffer, &data_stream->data, name_size * 2 + 2);

    /* iconv is the non-portable glibc way of doing it. */
    char *in_start = input_buffer, *out_start = file_name;
    size_t in_size = name_size * 2 + 2, out_size = name_size + 1;
    iconv_t conversion = iconv_open("UTF-8", "UTF-16LE");
    iconv(conversion, &in_start, &in_size, &out_start, &out_size);
    iconv_close(conversion);

    node *current = malloc(sizeof(node));
    current->key = name_hash;
    current->file_name = file_name;
    current->next = NULL;

    free(input_buffer);

    return current;
}
