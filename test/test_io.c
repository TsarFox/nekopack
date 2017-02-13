/* test_io.c -- MinUnit test cases for I/O related functions.

   Copyright (C) 2017 Jakob Tsar-Fox, All Rights Reserved.

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

#include <stdio.h>
#include <string.h>

#include "minunit.h"

#include "io.h"

int tests_run = 0;


static char *test_stream_obj(void) {
    struct stream *s_1 = stream_new(0x10);
    struct stream *s_2 = stream_new(0x20);
    struct stream *s_3 = stream_new(0x40);
    mu_assert("Incorrect size for `s_1`", s_1.len == 0x10);
    mu_assert("Incorrect size for `s_2`", s_1.len == 0x20);
    mu_assert("Incorrect size for `s_3`", s_1.len == 0x40);
    stream_free(s_1);
    stream_free(s_2);
    stream_free(s_3);
}


static char *test_stream_rw(void) {
    char dest[4];
    struct stream *s_1 = stream_new(0x4);
    stream_write(s_1, "\x00\x01\x02\x03", 4);
    mu_assert("Write failure", !memcmp(s_1->_start, "\x00\x01\x02\x03", 4));
    mu_assert("Cursor not incremented", s_1->cur - s_1->start == 4);
    stream_read(dest, s, 4);
    mu_assert("Read failure", !memcmp(dest, "\x00\x01\x02\x03", 4));
    stream_free(s_1);
}
