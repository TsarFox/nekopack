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

#define ELIF_MAGIC 0x46696c65
#define FILE_MAGIC 0x656c6946
#define ADLR_MAGIC 0x726c6461
#define SEGM_MAGIC 0x6d676573
#define INFO_MAGIC 0x6f666e69
#define TIME_MAGIC 0x656d6974

/* A pointer to the start of the memory region
   has to be kept for freeing purposes. */
typedef struct {
    uint64_t stream_length;
    Bytef *start;
    Bytef *data;
} memory_stream;

/* typedef struct { */
/*     uint64_t timestamp; */
/*     uint32_t hash_key; */
/* } file_entry; */

memory_stream decompress_stream(FILE *archive, uint64_t sizes_offset);
void read_stream(void *destination, Bytef **source, size_t size);
void read_file_entry(memory_stream *data_stream, Bytef *section_end);
void read_elif_entry(memory_stream *data_stream);
void read_info_chunk(memory_stream *data_stream);


/* Parses and extracts entries from the archive. */
void extract(FILE *archive, uint64_t table_offset) {
    /* An 8-bit unsigned integers at the table offset indicates
       whether or not the archive has to be decompressed. */
    uint8_t compressed;
    fseek(archive, table_offset, SEEK_SET);
    fread(&compressed, sizeof(uint8_t), 1, archive);

    memory_stream data_stream;
    if (compressed) {
        data_stream = decompress_stream(archive, ftell(archive));
    } else {
        fprintf(stderr, "Uncompressed archives not yet supported.\n");
        exit(EXIT_FAILURE);
    }

    /* The header for every entry in the XP3 archive format contains
       a magic number, followed by the size of the entry. */
    uint32_t entry_magic;
    uint64_t entry_size;
    for (;;) {
        read_stream(&entry_magic, &data_stream.data, sizeof(uint32_t));
        read_stream(&entry_size, &data_stream.data, sizeof(uint64_t));
        printf("entry at 0x%lx\n", data_stream.data - data_stream.start - 12);
        printf("Magic: %" PRIx32 " Size: %" PRIx64 "\n", entry_magic, entry_size); // Debug.

        switch (entry_magic) {
            case ELIF_MAGIC:
                /* The size given by the entry header doesn't match the
                   actual entry size, so it isn't passed to the function. */
                read_elif_entry(&data_stream);
                break;
            case FILE_MAGIC:
                // printf("[File entry]\n");
                // data_stream.data += entry_size;
                read_file_entry(&data_stream, data_stream.data + entry_size);
                break;
            default: // Debug.
                printf("Unknown magic: %x\n", entry_magic);
                exit(EXIT_FAILURE);
        }
    }

    free(data_stream.start);
}


/* Inflates the file pointer and returns a struct containing the
   size and a pointer to the decompressed data in memory. */
memory_stream decompress_stream(FILE *archive, uint64_t sizes_offset) {
    uint64_t compressed_size, decompressed_size;
    fseek(archive, sizes_offset, SEEK_SET);
    fread(&compressed_size, sizeof(uint64_t), 1, archive);
    fread(&decompressed_size, sizeof(uint64_t), 1, archive);

    /* Decompression is done in memory because it's $CURRENT_YEAR. */
    Bytef *compressed_data = malloc(compressed_size);
    Bytef *decompressed_data = malloc(decompressed_size);

    /* This is a pretty shitty way of handling it, though. */
    if (compressed_data == NULL || decompressed_data == NULL) {
        if (compressed_data != NULL)
            free(compressed_data);
        fprintf(stderr, "Insufficient memory to decompress archive.\n");
        fclose(archive);
        exit(EXIT_FAILURE);
    }

    z_stream data_stream;
    data_stream.zalloc = Z_NULL;
    data_stream.zfree = Z_NULL;
    data_stream.opaque = Z_NULL;
    data_stream.avail_in = 0;
    data_stream.next_in = Z_NULL;

    if (inflateInit(&data_stream) != Z_OK) {
        fprintf(stderr, "Could not initialize zlib.\n");
        free(compressed_data);
        free(decompressed_data);
        fclose(archive);
        exit(EXIT_FAILURE);
    }

    int status_code;
    do {
        fread(compressed_data, compressed_size, 1, archive);
        data_stream.avail_in = compressed_size;

        /* This really shouldn't happen. */
        if (ferror(archive)) {
            fprintf(stderr, "File corrupt.\n");
            inflateEnd(&data_stream);
            free(compressed_data);
            free(decompressed_data);
            exit(EXIT_FAILURE);
        }

        if (data_stream.avail_in == 0)
            break;
        data_stream.next_in = compressed_data;
        // Flushing probably isn't required here.
        do {
            data_stream.avail_out = decompressed_size;
            data_stream.next_out = decompressed_data;
            status_code = inflate(&data_stream, Z_NO_FLUSH);
        } while (data_stream.avail_out == 0);
    } while (status_code != Z_STREAM_END);

    /* The compressed data is irrelevant at this point. */
    free(compressed_data);

    return (memory_stream) {decompressed_size, decompressed_data,
                            decompressed_data};
}


/* Wrapper for memcpy which increments the source operand by
   the amount of bytes read to simulate a file stream. */
void read_stream(void *destination, Bytef **source, size_t size) {
    memcpy(destination, *source, size);
    *source += size;
}


/* Document */
void read_file_entry(memory_stream *data_stream, Bytef *section_end) {
    uint32_t entry_magic;
    uint64_t entry_size;
    while (data_stream->data < section_end) {
        read_stream(&entry_magic, &data_stream->data, sizeof(uint32_t));
        read_stream(&entry_size, &data_stream->data, sizeof(uint64_t));
        switch (entry_magic) {
            case ADLR_MAGIC:
                printf("[ADLR found at 0x%lx]\n", data_stream->data - data_stream->start);
                data_stream->data += entry_size;
                break;
            case SEGM_MAGIC:
                printf("[SEGM found at 0x%lx]\n", data_stream->data - data_stream->start);
                data_stream->data += entry_size;
                break;
            case INFO_MAGIC:
                printf("[INFO found at 0x%lx]\n", data_stream->data - data_stream->start);
                read_info_chunk(data_stream);
                /* data_stream->data += entry_size; */
                break;
            case TIME_MAGIC:
                printf("[TIME found at 0x%lx]\n", data_stream->data - data_stream->start);
                data_stream->data += entry_size;
                break;
            default:
                printf("New magic discovered: %" PRIx32 " size: %" PRIx64 "\n", entry_magic, entry_size);  // Debug.
        }
    }
    printf("\n\n\n\n"); // Debug.
}


/* Document */
void read_elif_entry(memory_stream *data_stream) {
    uint16_t name_size;
    /* The first part of an ELIF entry is a 32-bit file name hash. */
    data_stream->data += sizeof(uint32_t);
    read_stream(&name_size, &data_stream->data, sizeof(uint16_t));

    char *input_buffer = malloc(name_size * 2);
    char *file_name = malloc(name_size);
    read_stream(input_buffer, &data_stream->data, name_size * 2);

    /* There seems to be an extra UTF-16 byte at the end of the entry. */
    data_stream->data += 2;

    printf("Filename (ASCII): ");
    for (int i = 0; i < name_size * 2; i++) {
        if (input_buffer[i] >= 0x20 && input_buffer[i] < 0x7f)
            printf("%c", input_buffer[i]);
    }
    printf("\n");

    free(input_buffer);
    free(file_name);
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
    printf("DECOMPRESSED_SIZE: %" PRIx64 "\n", decompressed_size);
    printf("COMPRESSED_SIZE: %" PRIx64 "\n", compressed_size);
    printf("FILENAME (HASH?): ");
    for (int i = 0; i < file_name_size * 2; i++) {
        if (file_name[i] >= 0x20 && file_name[i] < 0x7f)
            printf("%c", file_name[i]);
    }
    printf("\n");

    data_stream->data += 2;
}
