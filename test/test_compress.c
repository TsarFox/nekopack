/* test_compress.c -- MinUnit test cases for compress.c

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

#include <stdbool.h>

#include "compress.h"
#include "io.h"

#include "minunit.h"


extern int tests_run;

/* FIXME: This won't hold up for the future. Even if it works across
   different versions of zlib, if the compression rate is ever changed
   this test will fail. */

/* TODO: Replace with a proper stream comparison and put it in io.c? */
static bool streams_eq(struct stream *a, struct stream *b) {
    if (a->len != b->len)
        return false;
    for (unsigned long i = 0; i < a->len; i++) {
        if (a->_start[i] != b->_start[i])
            return false;
    }
    return true;
}


const char *test_compress(void) {
    struct stream *s        = stream_from_file("test/vectors/test.png");
    struct stream *expected = stream_from_file("test/vectors/test.bin");
    mu_assert("[compress] test_compress: Could not open test vectors",
              s != NULL && expected != NULL);

    struct stream *res = stream_deflate(s, s->len);
    mu_assert("[compress] test_compress: Expected compression not met",
              streams_eq(res, expected));

    stream_free(s);
    stream_free(expected);
    stream_free(res);
    return NULL;
}


const char *test_decompress(void) {
    struct stream *s        = stream_from_file("test/vectors/test.bin");
    struct stream *expected = stream_from_file("test/vectors/test.png");
    mu_assert("[compress] test_decompress: Could not open test vectors",
              s != NULL && expected != NULL);

    struct stream *res = stream_inflate(s, s->len, 176608);
    mu_assert("[compress] test_decompress: Expected compression not met",
              streams_eq(res, expected));

    stream_free(s);
    stream_free(expected);
    stream_free(res);
    return NULL;
}
