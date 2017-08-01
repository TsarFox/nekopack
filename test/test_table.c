/* test_table.c -- MinUnit test cases for table.c

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

#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "table.h"

#include "minunit.h"


const char *test_table_list(void) {
    struct table_entry *root = calloc(sizeof(struct table_entry), 1);
    struct table_entry *next = calloc(sizeof(struct table_entry), 1);
    next->key = 0xffffffff;
    entry_append(root, next);
    mu_assert("[table] test_table_list: Table entry not inserted",
              root->next->key == 0xffffffff);
    entry_free(root);
    return NULL;
}


const char *test_table_elif(void) {
    struct stream *s = stream_new(10);
    struct table_entry *root = calloc(sizeof(struct table_entry), 1);
    stream_write(s, "\xff\xff\xff\xff\x01\x00\x41\x00\x00\x00", 10);
    stream_rewind(s);
    read_elif(s, root);
    mu_assert("[table] test_table_elif: Table entry not inserted (eliF)",
              root->next != NULL);
    mu_assert("[table] test_table_elif: Filename handling failure",
              !strcmp(root->next->filename, "A"));
    free(root->next->filename);
    root->next->filename = NULL;
    stream_rewind(s);
    read_elif(s, root);
    mu_assert("[table] test_table_elif: Existing entry ignored (eliF)",
              root->next->next == NULL);
    entry_free(root);
    stream_free(s);
    return NULL;
}


const char *test_table_file(void) {
    struct stream *s = stream_new(0x100);
    struct table_entry *root = calloc(sizeof(struct table_entry), 1);
    stream_write(s, "\x61\x64\x6c\x72\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff"
                 "\xff\xff\x73\x65\x67\x6d\x1c\x00\x00\x00\x00\x00\x00\x00\x00"
                 "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                 "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x69\x6e\x66"
                 "\x6f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                 "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                 "\x00\x00\x00\x74\x69\x6d\x65\x00\x00\x00\x00\x00\x00\x00\x00"
                 "\x00\x00\x00\x00\x00\x00\x00\x00", 113);
    stream_rewind(s);
    read_file(s, root);
    mu_assert("[table] test_table_file: Table entry not inserted (eliF)",
              root->next != NULL);
    mu_assert("[table] test_table_file: Key read failure",
              root->next->key == 0xffffffff);
    root->next->ctime = 420;
    stream_rewind(s);
    read_file(s, root);
    mu_assert("[table] test_table_file: Existing entry ignored (eliF)",
              root->next->ctime == 0);
    return NULL;
}
