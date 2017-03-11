/* io.c -- Code for file and memory I/O.

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"


/* Allocates `len` bytes of non-zeroed memory and returns a new stream
   structure pointing to it. */
struct stream *stream_new(size_t len) {
    struct stream *new = malloc(sizeof(struct stream));
    new->len = len;
    new->_start = malloc(len);
    new->_cur = new->_start;
    new->_loc = HEAP;
    return new;
}


/* Copies `n` bytes from `s` into a new stream structure. */
struct stream *stream_clone(struct stream *s, size_t n) {
    struct stream *new = stream_new(n);
    stream_write(new, s->_cur, n);
    stream_rewind(new); /* New! This fixes the only bug we've had so far! */
    /* Look for other shit like this! */
    return new;
}


/* Maps the file at the given `path` into a stream structure. */
struct stream *stream_from_file(char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) return NULL;
    struct stream *new = malloc(sizeof(struct stream));
    fseek(fp, 0, SEEK_END);
    new->len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    new->_start = malloc(new->len);
    new->_cur = new->_start;
    fread(new->_start, new->len, 1, fp);
    new->_loc = HEAP;
    fclose(fp);
    return new;
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
   by `dest`. The stream's cursor is advanced appropriately. */
void stream_read(void *dest, struct stream *s, size_t n) {
    memcpy(dest, s->_cur, n);
    s->_cur += n;
}


/* Dumps the contents of `s` into the file specified by `fp`. */
void stream_dump(FILE *fp, struct stream *s, size_t n) {
    fwrite(s->_cur, n, 1, fp);
}


/* Copies `n` bytes into the given stream from the memory area specified
   by `src`. The stream's cursor is advanced appropriately. */
void stream_write(struct stream *s, void *src, size_t n) {
    if (s->_cur + n > s->_start + s->len) {
        ptrdiff_t dist = (uintptr_t) s->_cur - (uintptr_t) s->_start;
        switch(s->_loc) {
            case HEAP:
                s->len *= 2;
                s->_start = realloc(s->_start, s->len);
                s->_cur = s->_start + dist;
        }
    }
    memcpy(s->_cur, src, n);
    s->_cur += n;
}


/* Applies an initial and primary key to the given stream, effectively
   encrypting or decrypting it. */
void stream_xor(struct stream *s, uint8_t initial, uint8_t primary) {
    *s->_start ^= initial;
    for (char *p = s->_start; p < s->_start + s->len; *p++ ^= primary);
}


/* Obtains the current value of the stream's position indicator. */
size_t stream_tell(struct stream *s) {
    return s->_cur - s->_start;
}


/* Sets the stream's position indicator to the given `pos`. If `whence`
   is set to SEEK_SET, SEEK_CUR, or SEEK_END, the offset is relative to
   the start of the file, the current position indicator, or
   end-of-file, respectively. */
void stream_seek(struct stream *s, size_t pos, int whence) {
    switch (whence) {
        case SEEK_SET:
            s->_cur = s->_start + pos;
            break;
        case SEEK_CUR:
            s->_cur += pos;
            break;
        case SEEK_END:
            s->_cur = s->_start + s->len - pos;
    }
}


/* Sets the stream's position indicator to the beginning. */
void stream_rewind(struct stream *s) {
    s->_cur = s->_start;
}
