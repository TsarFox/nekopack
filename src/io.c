/* io.c -- Code for file and memory I/O.

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
#include <string.h>

#include "io.h"


/* Allocates `len` bytes of memory and returns a new stream pointing to
   it. The memory provided has not zeroed. */
struct stream *stream_new(size_t len) {
    stream *new = malloc(sizeof(stream));
    new->len = len;
    new->_start = malloc(len);
    new->_cur = new->_start;
    new->_location = HEAP;
}


/* Called to free or unmap the memory chunk associated with the given
   stream, as well as the stream structure itself. */
void stream_free(struct stream *s) {
    switch (s->_loc) {
        case HEAP:
            free(s->_start);
    }
    free(s);
}


/* Copies `n` bytes from the given stream into the memory area specified
   by `dest`. The stream is advanced appropriately. */
void stream_read(void *dest, struct stream *s, size_t n) {
    memcpy(dest, s->_cur, n);
    (char *) s->_cur += n;
}


/* Copies `n` bytes into the given stream from the memory area specified
   by `src`. The stream is advanced appropriately. */
void stream_write(struct stream *s, void *src, size_t n) { 
    if (s->cur + n > s->start + s->len) {
        ptrdiff_t dist = (uintptr_t) s->_cur - (uintptr_t) s->_start;
        switch(s->_loc) {
            case HEAP:
                s->_start = realloc(s->_start, s->len * 2);
                s->_cur = s->_start + dist;
        }
    }
    memcpy(s->cur, src, n);
    s->cur += n;
}
