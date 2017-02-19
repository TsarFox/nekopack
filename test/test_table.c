/* test_table.c -- MinUnit test cases for table.c

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

#include <stdlib.h>

#include "minunit.h"

#include "table.h"


char *test_table_list(void) {
    struct table_entry *root = calloc(sizeof(struct table_entry), 1);
    mu_assert("Structure not zeroed", root->next == NULL);
    struct table_entry *next = calloc(sizeof(struct table_entry), 1);
    next->key = 0xffffffff;
    entry_append(root, next);
    mu_assert("Entry not inserted", root->next->key == 0xffffffff);
    entry_free(root);
    return NULL;
}
