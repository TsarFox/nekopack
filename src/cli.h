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

/* Enumerable type representing Nekopara games, as they have different
   encryption keys. It's stored as an enum because multiple strcmp calls
   to figure out how to decrypt is a waste of processor cycles. */
typedef enum game {
    NO_CRYPTO,
    NEKOPARA_VOLUME_0,
    NEKOPARA_VOLUME_0_STEAM,
    NEKOPARA_VOLUME_1,
    NEKOPARA_VOLUME_1_STEAM,
} game;

/* Binary structure for storing command-line options. */
struct configuration {
    game source; /* Which decryption key to use. */
    const char *archive_path; /* Path to archive to extract. */
};

/* General subroutine for parsing command-line arguments. Returns a
   "configuration" structure containing everything parsed from argv. */
struct configuration parse_args(int argc, char *argv[]);
