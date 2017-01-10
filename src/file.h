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

#include <stdint.h>

/* Structure representing a 28-byte segment in a file. */
typedef struct {
    uint32_t compressed; /* Whether or not the chunk is compressed. */
    uint64_t offset; /* Chunk's position in the file as an offset. */
    uint64_t compressed_size; /* Chunk's compressed size. */
    uint64_t decompressed_size; /* Chunk's decompressed size. */
} segment;

/* Node in a linked list of file entries to write to disk. */
typedef struct file_node {
    int compressed; /* Whether or not the archive is compressed. */
    uint32_t key; /* Key associated with matching eliF entry. */
    uint64_t compressed_size; /* Size of compressed chunk. */
    uint64_t decompressed_size; /* Size of decompressed data. */
    uint64_t segment_count; /* Number of segments in File entry. */
    segment **segments; /* Data segments associated with the entry. */
    struct file_node *next; /* Pointer to the next node. */
} file_node;

/* Creates a file node by parsing a file entry. */
file_node *read_file_node(memory_stream *data_stream, Bytef *section_end);
