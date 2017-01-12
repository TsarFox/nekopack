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

#include <zlib.h>

#include "extract.h"

/* Decompresses a memory_stream data structure into a new one. */
memory_stream decompress_stream(memory_stream compressed_data, uint64_t size);

/* Reads a FILE pointer into a memory_stream data structure. */
memory_stream read_to_stream(FILE *archive, uint64_t buffer_size);

/* Returns a pointer to a buffer containing the
   inflated contents of a given memory chunk. */
Bytef *inflate_chunk(Bytef *chunk, uint64_t chunk_size,
                     uint64_t decompressed_size);
