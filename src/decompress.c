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


/* Inflates a chunk into new space in memory. NULL is returned if not
   enough memory can be allocated to contain the decompressed data. */
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
        do {
            data_stream.avail_out = decompressed_size;
            data_stream.next_out = decompressed_data;
            status_code = inflate(&data_stream, Z_NO_FLUSH);
        } while (data_stream.avail_out == 0);
    } while (status_code != Z_STREAM_END);

    inflateEnd(&data_stream);
    return decompressed_data;
}


/* Decompresses the contents of a memory_stream data structure.
   Program exits if not enough memory can be allocated. */
memory_stream decompress_stream(memory_stream compressed_data, uint64_t size) {
    uint64_t chunk_size = compressed_data.stream_length;
    Bytef *decompressed_data, *chunk = compressed_data.start;

    decompressed_data = inflate_chunk(chunk, chunk_size, size);
    if (decompressed_data == NULL) {
        free(chunk);
        exit(EXIT_FAILURE);
    }

    return (memory_stream) {size, decompressed_data, decompressed_data};
}


/* Reads a FILE pointer into a memory_stream data structure. */
memory_stream read_to_stream(FILE *archive, uint64_t buffer_size) {
    Bytef *buffer = malloc(buffer_size);
    /* This is a pretty shitty way of handling it. */
    if (buffer == NULL) {
        fprintf(stderr, "Insufficient memory to load archive.\n");
        free(buffer);
        fclose(archive);
        exit(EXIT_FAILURE);
    }
    fread(buffer, buffer_size, 1, archive);
    return (memory_stream) {buffer_size, buffer, buffer};
}
