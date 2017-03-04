/* test_crypto.c -- MinUnit test cases for crypto.c

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

#include <stddef.h>
#include <stdint.h>

#include "minunit.h"

#include "crypto.h"

extern int tests_run;


char *test_derive_initial(void) {
    struct game_key k = get_key(NEKOPARA_VOLUME_0);
    mu_assert("Initial key failure", derive_initial(k, 0xdeadbeefcafebabe) == 0x22);
    return NULL;
}


char *test_derive_primary(void) {
    struct game_key k = get_key(NEKOPARA_VOLUME_1);
    mu_assert("Initial key failure", derive_primary(k, 0xdeadbeefcafebabe) == 0x13);
    return NULL;
}
