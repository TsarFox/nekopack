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

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "extract.h"
#include "file.h"

#define ADLR_MAGIC 0x726c6461
#define SEGM_MAGIC 0x6d676573
#define INFO_MAGIC 0x6f666e69
#define TIME_MAGIC 0x656d6974

void read_info_chunk(memory_stream *data_stream, file_node *parsed);
void read_adlr_chunk(memory_stream *data_stream, file_node *parsed);
void read_time_chunk(memory_stream *data_stream, file_node *parsed);
void read_segm_chunk(memory_stream *data_stream, file_node *parsed,
                     uint64_t segment_count);


/* Creates a file node by parsing a file entry. */
file_node *read_file_entry(memory_stream *data_stream, Bytef *section_end) {
    uint32_t entry_magic;
    uint64_t entry_size;
    file_node *parsed = calloc(sizeof(file_node), 1);
    while (data_stream->data < section_end) {
        read_stream(&entry_magic, &data_stream->data, sizeof(uint32_t));
        read_stream(&entry_size, &data_stream->data, sizeof(uint64_t));
        switch (entry_magic) {
            case ADLR_MAGIC:
                read_adlr_chunk(data_stream, parsed);
                break;
            case SEGM_MAGIC:
                /* Segments are 28 bytes each. */
                read_segm_chunk(data_stream, parsed, entry_size / 28);
                break;
            case INFO_MAGIC:
                read_info_chunk(data_stream, parsed);
                break;
            case TIME_MAGIC:
                read_time_chunk(data_stream, parsed);
                break;
            default:
                printf("Unknown magic: %" PRIx32 "\n", entry_magic);
        }
    }
    return parsed;
}


/* Reads the contents of an info chunk int a file_node. */
void read_info_chunk(memory_stream *data_stream, file_node *parsed) {
    uint32_t encrypted;
    uint64_t decompressed_size, compressed_size;
    uint16_t file_name_size;
    read_stream(&encrypted, &data_stream->data, sizeof(uint32_t));
    read_stream(&decompressed_size, &data_stream->data, sizeof(uint64_t));
    read_stream(&compressed_size, &data_stream->data, sizeof(uint64_t));
    read_stream(&file_name_size, &data_stream->data, sizeof(uint16_t));
    parsed->encrypted = encrypted;

    char *file_name = malloc(file_name_size * 2 + 2);
    read_stream(file_name, &data_stream->data, file_name_size * 2 + 2);
}


/* Reads the contents of a segm chunk into a file_node. */
void read_segm_chunk(memory_stream *data_stream, file_node *parsed,
                     uint64_t segment_count) {
    /* Segments are stored in an array of segment pointers. */
    segment **segments = malloc(sizeof(segment *) * segment_count);
    for (uint64_t i = 0; i < segment_count; i++) {
        segments[i] = malloc(sizeof(segment));
        read_stream(&segments[i]->compressed,
                    &data_stream->data,
                    sizeof(uint32_t));
        read_stream(&segments[i]->offset,
                    &data_stream->data,
                    sizeof(uint64_t));
        read_stream(&segments[i]->decompressed_size,
                    &data_stream->data,
                    sizeof(uint64_t));
        read_stream(&segments[i]->compressed_size,
                    &data_stream->data,
                    sizeof(uint64_t));
        parsed->file_size += segments[i]->decompressed_size;
    }
    parsed->segment_count = segment_count;
    parsed->segments = segments;
}


/* Reads the contents of an adlr chunk into a file_node. */
void read_adlr_chunk(memory_stream *data_stream, file_node *parsed) {
    uint32_t key;
    read_stream(&key, &data_stream->data, sizeof(uint32_t));
    parsed->key = key;
}


/* Reads the contents of a time chunk into a file_node. */
void read_time_chunk(memory_stream *data_stream, file_node *parsed) {
    uint64_t timestamp;
    read_stream(&timestamp, &data_stream->data, sizeof(uint64_t));
}
