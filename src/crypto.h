/* crypto.h -- Code for deriving encryption keys.

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

#include <stdbool.h>
#include <stdint.h>

/* Game identifiers for use with get_key. */
enum {
    NONE,
    NEKOPARA_VOLUME_0,
    NEKOPARA_VOLUME_0_STEAM,
    NEKOPARA_VOLUME_1,
    NEKOPARA_VOLUME_1_STEAM,
};

/* Structure containing game-specific encryption information. */
struct game_key {
    bool     uses_initial;      /* Whether or not to use an initial key. */
    uint32_t master;            /* Master key specific to the game. */
    uint8_t  fallback_initial;  /* Fallback if initial key is 0. */
    uint8_t  fallback_primary;  /* Fallback if primary key is 0. */
};

/* Returns the game_key structure for a given game identifier, as
   defined in crypto.h */
struct game_key get_key(int game);

/* Generates an initial key for the given key structure and file key. */
uint8_t derive_initial(struct game_key k, uint64_t file_key);

/* Generates a primary key for the given key structure and file key. */
uint8_t derive_primary(struct game_key k, uint64_t file_key);
