/* test_io.c -- MinUnit test cases for io.c

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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "minunit.h"

#include "io.h"

extern int tests_run;


char *test_stream_obj(void) {
    struct stream *s_1 = stream_new(0x10);
    struct stream *s_2 = stream_new(0x20);
    struct stream *s_3 = stream_new(0x40);
    mu_assert("Incorrect size for stream `s_1`", s_1->len == 0x10);
    mu_assert("Incorrect size for stream `s_2`", s_2->len == 0x20);
    mu_assert("Incorrect size for stream `s_3`", s_3->len == 0x40);
    stream_free(s_1);
    stream_free(s_2);
    stream_free(s_3);
    return NULL;
}


char *test_stream_rw(void) {
    char dest[4];
    struct stream *s = stream_new(0x4);
    stream_write(s, "\x00\x01\x02\x03", 4);
    mu_assert("Stream write failure", !memcmp(s->_start, "\x00\x01\x02\x03", 4));
    mu_assert("Stream write did not increment cursor", s->_cur - s->_start == 4);
    s->_cur = s->_start;

    stream_read(dest, s, 4);
    mu_assert("Stream read failure", !memcmp(dest, "\x00\x01\x02\x03", 4));
    mu_assert("Stream read did not increment cursor", s->_cur - s->_start == 4);
    stream_free(s);
    return NULL;
}


char *test_stream_dump(void) {
    char buf[4];
    FILE *fp = tmpfile();
    struct stream *s = stream_new(0x4);
    stream_write(s, "\x00\x01\x02\x03", 4);
    stream_rewind(s);
    stream_dump(fp, s, 4);
    fseek(fp, 0, SEEK_SET);
    fread(buf, 4, 1, fp);
    mu_assert("Stream dump failure", !memcmp(buf, "\x00\x01\x02\x03", 4));
    fclose(fp);
    return NULL;
}


char *test_stream_realloc(void) {
    struct stream *s = stream_new(0x2);
    stream_write(s, "\x00\x01\x02\x03", 4);
    mu_assert("Stream realloc did not modify `len`", s->len > 0x2);
    return NULL;
}


char *test_stream_xor(void) {
    struct stream *s = stream_new(2);
    stream_write(s, "\x01\x01", 2);
    stream_xor(s, 1, 0);
    mu_assert("Stream initial key failure", !memcmp(s->_start, "\x00\x01", 2));
    stream_xor(s, 0, 1);
    mu_assert("Stream primary key failure", !memcmp(s->_start, "\x01\x00", 2));
    stream_free(s);
    return NULL;
}


char *test_stream_nav(void) {
    struct stream *s = stream_new(2);
    mu_assert("Stream cursor not at beginning", stream_tell(s) == 0);
    stream_seek(s, 2, SEEK_CUR);
    mu_assert("Stream cursor not advanced (SEEK_CUR)", stream_tell(s) == 2);
    stream_seek(s, 2, SEEK_SET);
    mu_assert("Stream cursor not advanced (SEEK_SET)", stream_tell(s) == 2);
    stream_seek(s, 0, SEEK_END);
    mu_assert("Stream cursor not advanced (SEEK_END)", stream_tell(s) == 2);
    stream_rewind(s);
    mu_assert("Stream cursor not rewinded", stream_tell(s) == 0);
    stream_free(s);
    return NULL;
}
