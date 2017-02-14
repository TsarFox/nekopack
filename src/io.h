/* io.h -- Code for file and memory I/O.

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

#pragma once

#include <stdbool.h>
#include <stddef.h>

/* Internal enumerable type for representing the location of a memory
   segment. Used for freeing and unmapping purposes, if necessary. */
enum _location {
    HEAP,
};

/* Structure to emulate a stream of data that maintains a current
   position. The structure additionally maintains a `length` variable
   containing the stream's length. */
struct stream {
    size_t         len; /* Length of the stream. */
    char           *_start; /* Pointer to the stream's start. */
    char           *_cur; /* Pointer to the current position. */
    enum _location _loc; /* Location of the stream in memory. */
};

/* Allocates `len` bytes of non-zeroed memory and returns a new stream
   structure pointing to it. */
struct stream *stream_new(size_t len);

/* Called to free or unmap the memory chunk associated with the given
   stream, as well as the stream structure itself. */
void stream_free(struct stream *s);

/* Copies `n` bytes from the given stream into the memory area specified
   by `dest`. The stream's cursor is advanced appropriately. */
void stream_read(void *dest, struct stream *s, size_t n);

/* Copies `n` bytes into the given stream from the memory area specified
   by `src`. The stream's cursor is advanced appropriately. */
void stream_write(struct stream *s, void *src, size_t n);
