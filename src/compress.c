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

#include <zlib.h>

#include "io.h"


/* Inflates `s` into a newly allocated stream structure. */
struct stream *stream_inflate(struct stream *s, size_t len,
                              size_t decompressed_len) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    if (inflateInit(&strm) != Z_OK)
        return NULL;

    int ret;
    struct stream *n = stream_new(decompressed_len);

    do {
        strm.avail_in = len;
        strm.next_in = (Bytef *) s->_cur;
        do {
            strm.avail_out = decompressed_len;
            strm.next_out = (Bytef *) n->_cur;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR) {
                stream_free(n);
                inflateEnd(&strm);
                return NULL;
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    return n;
}
