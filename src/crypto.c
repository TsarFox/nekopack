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

#include "cli.h"
#include "crypto.h"


/* Returns the encryption keys for a given game value. */
key get_encryption_key(game_type game) {
    key encryption_key;
    switch (game) {
        case NEKOPARA_VOLUME_0:
            encryption_key.master_key = 0x1548e29c;
            encryption_key.initial_fallback_key = 0x9c;
            encryption_key.primary_fallback_key = 0xd7;
            encryption_key.uses_initial_key = 1;
            break;
        case NEKOPARA_VOLUME_0_STEAM:
            encryption_key.master_key = 0x44528b87;
            encryption_key.initial_fallback_key = 0x87;
            encryption_key.primary_fallback_key = 0x23;
            encryption_key.uses_initial_key = 1;
            break;
        case NEKOPARA_VOLUME_1:
            encryption_key.master_key = 0x1548e29c;
            encryption_key.initial_fallback_key = 0x00;
            encryption_key.primary_fallback_key = 0xd7;
            encryption_key.uses_initial_key = 0;
            break;
        case NEKOPARA_VOLUME_1_STEAM:
            encryption_key.master_key = 0x44528b87;
            encryption_key.initial_fallback_key = 0x00;
            encryption_key.primary_fallback_key = 0x23;
            encryption_key.uses_initial_key = 0;
            break;
        default:
            encryption_key.master_key = 0x00000000;
            encryption_key.initial_fallback_key = 0x00;
            encryption_key.primary_fallback_key = 0x00;
            encryption_key.uses_initial_key = 0;
    }
    return encryption_key;
}


/* Decrypts the contents of a buffer according to a file key. */
void decrypt_buffer(Bytef *encrypted_buffer, uint64_t buffer_length,
                    key encryption_key, uint32_t file_key) {
    uint32_t xor_key = file_key ^ encryption_key.master_key;
    uint8_t initial_key = xor_key & 0xff;
    uint8_t primary_key = (xor_key >> 24 ^ xor_key >> 16 ^ \
                           xor_key >> 8 ^ xor_key) & 0xff;
    if (xor_key == 1 && initial_key == 0)
        initial_key = encryption_key.initial_fallback_key;
    else if (primary_key == 0)
        primary_key = encryption_key.primary_fallback_key;
    if (encryption_key.uses_initial_key)
        encrypted_buffer[0] ^= initial_key;
    for (uint64_t i = 0; i < buffer_length; i++) {
        encrypted_buffer[i] ^= primary_key;
    }
}
