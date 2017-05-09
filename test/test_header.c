/* test_header.c -- MinUnit test cases for header.c

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
#include <string.h>

#include "minunit.h"

#include "header.h"
#include "io.h"

extern int tests_run;


char *test_header_read(void) {
    struct stream *s = stream_new(sizeof(struct header));
    memset(s->_start, '\x00', sizeof(struct header));

    struct header *h = read_header(s);
    mu_assert("Header assertions failed", h == NULL);

    stream_rewind(s);
    memcpy(s->_start, "\x58\x50\x33\x0d\x0a\x20\x0a\x1a\x8b\x67\x01\x17\x00"
           "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x80\x00\x00\x00\x00\x00"
           "\x00\x00\x00\xd1\xfe\x56\x0b\x00\x00\x00\x00", 42);
    h = read_header(s);
    mu_assert("Header parsing failure", h != NULL);

    stream_free(s);
    return NULL;
}


char *test_header_creation(void) {
    struct header *h = create_header();
    mu_assert("Header allocation failed", h != NULL);
    mu_assert("Incorrect XP3 header", !memcmp(h->magic, XP3_MAGIC, 11));
    return NULL;
}
