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

#define ADLR_MAGIC 0x726c6461
#define SEGM_MAGIC 0x6d676573
#define INFO_MAGIC 0x6f666e69
#define TIME_MAGIC 0x656d6974

void read_info_chunk(memory_stream *data_stream);
void read_segm_chunk(memory_stream *data_stream);
void read_adlr_chunk(memory_stream *data_stream);
void read_time_chunk(memory_stream *data_stream);


/* Document and update documentation in header file. */
void read_file_entry(memory_stream *data_stream, Bytef *section_end) {
    uint32_t entry_magic;
    uint64_t entry_size;
    while (data_stream->data < section_end) {
        read_stream(&entry_magic, &data_stream->data, sizeof(uint32_t));
        read_stream(&entry_size, &data_stream->data, sizeof(uint64_t));
        switch (entry_magic) {
            case ADLR_MAGIC:
                printf("[ADLR found at 0x%lx]\n", data_stream->data - data_stream->start);
                read_adlr_chunk(data_stream);
                break;
            case SEGM_MAGIC:
                printf("[SEGM found at 0x%lx]\n", data_stream->data - data_stream->start);
                read_segm_chunk(data_stream);
                break;
            case INFO_MAGIC:
                printf("[INFO found at 0x%lx]\n", data_stream->data - data_stream->start);
                read_info_chunk(data_stream);
                break;
            case TIME_MAGIC:
                printf("[TIME found at 0x%lx]\n", data_stream->data - data_stream->start);
                read_time_chunk(data_stream);
                break;
            default:
                printf("New magic discovered: %" PRIx32 " size: %" PRIx64 "\n", entry_magic, entry_size);  // Debug.
        }
    }
    printf("\n\n\n\n"); // Debug.
}


/* Document */
void read_info_chunk(memory_stream *data_stream) {
    uint32_t flags;
    uint64_t decompressed_size, compressed_size;
    uint16_t file_name_size;
    read_stream(&flags, &data_stream->data, sizeof(uint32_t));
    read_stream(&decompressed_size, &data_stream->data, sizeof(uint64_t));
    read_stream(&compressed_size, &data_stream->data, sizeof(uint64_t));
    read_stream(&file_name_size, &data_stream->data, sizeof(uint16_t));

    char *file_name = malloc(file_name_size * 2);
    read_stream(file_name, &data_stream->data, file_name_size * 2);
    printf("\nINFO SEGMENT\n");
    printf("------------\n");
    printf("FLAGS: %" PRIx32 "\n", flags);
    printf("COMPRESSED_SIZE: %" PRIx64 "\n", compressed_size);
    printf("DECOMPRESSED_SIZE: %" PRIx64 "\n", decompressed_size);
    printf("MD5: ");
    for (int i = 0; i < file_name_size * 2; i++) {
        if (file_name[i] >= 0x20 && file_name[i] < 0x7f)
            printf("%c", file_name[i]);
    }
    printf("\n");

    data_stream->data += 2;
}


/* Document */
void read_segm_chunk(memory_stream *data_stream) {
    uint32_t flags;
    uint64_t offset, compressed_size, decompressed_size;
    read_stream(&flags, &data_stream->data, sizeof(uint32_t));
    read_stream(&offset, &data_stream->data, sizeof(uint64_t));
    read_stream(&compressed_size, &data_stream->data, sizeof(uint64_t));
    read_stream(&decompressed_size, &data_stream->data, sizeof(uint64_t));
    printf("\nSEGM SEGMENT\n");
    printf("------------\n");
    printf("FLAGS: %" PRIx32 "\n", flags);
    printf("MEM_OFFSET: %" PRIx64 "\n", offset);
    printf("DECOMPRESSED_SIZE: %" PRIx64 "\n", decompressed_size);
    printf("COMPRESSED_SIZE: %" PRIx64 "\n\n", compressed_size);
    /* Entries can be between the segment chunk and the actual file
       data, so processing the file has to be deferred for later. */
}


/* Document */
void read_adlr_chunk(memory_stream *data_stream) {
    uint32_t key;
    read_stream(&key, &data_stream->data, sizeof(uint32_t));
    printf("\nADLR SEGMENT\n");
    printf("------------\n");
    printf("KEY: %" PRIx32 "\n\n", key);
}


/* Document */
void read_time_chunk(memory_stream *data_stream) {
    uint64_t timestamp;
    read_stream(&timestamp, &data_stream->data, sizeof(uint64_t));
    printf("\nTIME SEGMENT\n");
    printf("------------\n");
    printf("TIMESTAMP: %" PRIx64 "\n\n", timestamp);
}
