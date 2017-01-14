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

#pragma once

#include <stdio.h>
#include <stdint.h>

#include <zlib.h>

/* Structure representing a "stream" in memory. A pointer to the
   start of the memory region is kept for freeing purposes. */
typedef struct {
    uint64_t stream_length;
    Bytef *start;
    Bytef *data;
} memory_stream;

/* Decrypts and writes files in the XP3 archive
   to disk according to table entries. */
void extract(memory_stream data_stream, FILE *archive);

/* Simply lists the contents of an archive, ignoring File entries. */
void list(memory_stream data_stream);

/* Wrapper for memcpy which increments the source operand by
   the amount of bytes read to simulate a file stream. */
void read_stream(void *destination, Bytef **source, size_t size);
