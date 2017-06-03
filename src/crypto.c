/* crypto.c -- Code for deriving encryption keys.

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

#include <stdbool.h>
#include <stdint.h>

#include "crypto.h"


/* Returns the game_key structure for a given game identifier, as
   defined in crypto.h */
struct game_key get_key(int game) {
    struct game_key k = {0};
    switch (game) {
    case NEKOPARA_VOLUME_0:
        k.master           = 0x1548e29c;
        k.fallback_initial = 0x9c;
        k.fallback_primary = 0xd7;
        k.uses_initial     = true;
        break;
    case NEKOPARA_VOLUME_0_STEAM:
        k.master           = 0x44528b87;
        k.fallback_initial = 0x87;
        k.fallback_primary = 0x23;
        k.uses_initial     = true;
        break;
    case NEKOPARA_VOLUME_1:
        k.master           = 0x1548e29c;
        k.fallback_initial = 0x00;
        k.fallback_primary = 0xd7;
        k.uses_initial     = false;
        break;
    case NEKOPARA_VOLUME_1_STEAM:
        k.master           = 0x44528b87;
        k.fallback_initial = 0x00;
        k.fallback_primary = 0x23;
        k.uses_initial     = false;
        break;
    }
    return k;
}


/* Generates an initial key for the given key structure and file key. */
uint8_t derive_initial(struct game_key k, uint64_t file_key) {
    if (!k.uses_initial) return 0x00;
    uint8_t initial = (k.master ^ file_key) & 0xff;
    return initial == 0 ? k.fallback_initial : initial;
}


/* Generates a primary key for the given key structure and file key. */
uint8_t derive_primary(struct game_key k, uint64_t file_key) {
    if (!k.master) return 0x00;
    uint32_t tmp     = k.master ^ file_key;
    uint8_t  primary = (tmp >> 24 ^ tmp >> 16 ^ tmp >> 8 ^ tmp) & 0xff;
    return primary == 0 ? k.fallback_primary : primary;
}
