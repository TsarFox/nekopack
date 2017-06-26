/* compress.c -- Wrappers for compression and decompression with zlib.

   Copyright (C) 2017 Jakob Kreuze, All Rights Reserved.

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

#include <stdlib.h>

#include <zlib.h>

#include "io.h"

#define LEVEL -1


/* Inflates `len` bytes from the current position of `s` to a new stream
   structure of size `decompressed_len` bytes. Stream inflation will not
   work if `decompressed_len` does not represent the actual size of the
   original data. */
struct stream *stream_inflate(struct stream *s, size_t len,
                              size_t decompressed_len) {
    z_stream strm;
    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = 0;
    strm.next_in  = Z_NULL;

    if (inflateInit(&strm) != Z_OK)
        return NULL;

    int ret;
    struct stream *new = stream_new(decompressed_len);
    if (new == NULL)
        return NULL;

    do {
        strm.avail_in = len;
        strm.next_in = (Bytef *) s->_cur;
        do {
            strm.avail_out = decompressed_len;
            strm.next_out  = (Bytef *) new->_cur;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR) {
                stream_free(new);
                inflateEnd(&strm);
                return NULL;
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    return new;
}


/* Deflates `len` bytes from the current position of `s` to a new stream
   structure, where the `len` member represents the decompressed size. */
struct stream *stream_deflate(struct stream *s, size_t len) {
    struct stream *new = stream_new(len);
    if (new == NULL)
        return NULL;

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;

    if (deflateInit(&strm, LEVEL) != Z_OK)
        return NULL;

    strm.avail_in = len;
    strm.next_in  = (unsigned char *) s->_cur;
    do {
        strm.avail_out = len;
        strm.next_out  = (unsigned char *) new->_cur;
        deflate(&strm, Z_FINISH);

        /* This means that the compressed data is larger than the
           decompressed, and that the buffer needs to be expanded. */
        if (strm.avail_out == 0) {
            new->_cur += len;
            stream_expand(new, s->len);
        }
    } while (strm.avail_out == 0);

    new->len -= strm.avail_out;
    new->_cur = new->_start;

    deflateEnd(&strm);
    return new;
}
