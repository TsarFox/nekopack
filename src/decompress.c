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
#include <stdlib.h>

#include <zlib.h>

#include "defs.h"
#include "extract.h"


/* Returns a pointer to a buffer containing the
   inflated contents of a given memory chunk. */
Bytef *inflate_chunk(Bytef *chunk, uint64_t chunk_size,
                     uint64_t decompressed_size) {
    Bytef *decompressed_data = malloc(decompressed_size);

    if (decompressed_data == NULL) {
        fprintf(stderr, "Insufficient memory to decompress archive.\n");
        free(decompressed_data);
        return NULL;
    }

    z_stream data_stream;
    data_stream.zalloc = Z_NULL;
    data_stream.zfree = Z_NULL;
    data_stream.opaque = Z_NULL;
    data_stream.avail_in = 0;
    data_stream.next_in = Z_NULL;

    if (inflateInit(&data_stream) != Z_OK) {
        fprintf(stderr, "Could not initialize zlib.\n");
        free(decompressed_data);
        return NULL;
    }

    int status_code;
    do {
        data_stream.avail_in = chunk_size;
        if (data_stream.avail_in == 0)
            break;
        data_stream.next_in = chunk;
        // Flushing probably isn't required here.
        do {
            data_stream.avail_out = decompressed_size;
            data_stream.next_out = decompressed_data;
            status_code = inflate(&data_stream, Z_NO_FLUSH);
        } while (data_stream.avail_out == 0);
    } while (status_code != Z_STREAM_END);

    inflateEnd(&data_stream);

    return decompressed_data;
}


/* Wrapper for inflate_chunk which operates on FILE pointers. The file
   pointer's contents are inflated into a memory stream and returned. */
memory_stream decompress_file(FILE *archive, uint64_t sizes_offset) {
    uint64_t compressed_size, decompressed_size;
    fseek(archive, sizes_offset, SEEK_SET);
    fread(&compressed_size, sizeof(uint64_t), 1, archive);
    fread(&decompressed_size, sizeof(uint64_t), 1, archive);

    /* Decompression is done in memory because it's $CURRENT_YEAR. */
    Bytef *decompressed_data, *compressed_data = malloc(compressed_size);

    /* This is a pretty shitty way of handling it, though. */
    if (compressed_data == NULL) {
        fprintf(stderr, "Insufficient memory to decompress archive.\n");
        free(compressed_data);
        fclose(archive);
        exit(EXIT_FAILURE);
    }

    fread(compressed_data, compressed_size, 1, archive);
    decompressed_data = inflate_chunk(compressed_data, compressed_size,
                                      decompressed_size);
    /* The compressed data is irrelevant at this point. */
    free(compressed_data);
    if (decompressed_data == NULL)
        exit(EXIT_FAILURE);

    return (memory_stream) {decompressed_size, decompressed_data,
                            decompressed_data};
}
