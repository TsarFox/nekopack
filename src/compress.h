/* compress.h -- Wrappers for compression and decompression with zlib.

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

#pragma once

#include "io.h"

/* Inflates `s` into a newly allocated stream structure. */
struct stream *stream_inflate(struct stream *s, size_t len,
                              size_t decompressed_len);

/* Deflates `s` into a newly allocated stream structure. */
struct stream *stream_deflate(struct stream *s, size_t len);
