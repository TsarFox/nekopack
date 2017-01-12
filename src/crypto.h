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

#include <zlib.h>

#include "cli.h"

/* Struct for a game's encryption keys.  */
typedef struct {
    int uses_initial_key; /* Set if first byte uses a different key. */
    uint32_t master_key; /* Master key used to derive a key. */
    uint8_t initial_fallback_key; /* Fallback for the initial key. */
    uint8_t primary_fallback_key; /* Fallback for the primary key. */
} key;

/* Returns the encryption keys for a given game value. */
key get_encryption_key(game current_game);

/* Decrypts the contents of a buffer according to a file key. */
void decrypt_buffer(Bytef *encrypted_buffer, uint64_t buffer_length,
                    key encryption_key, uint32_t file_key);
