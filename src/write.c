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

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "write.h"


/* Document */
char *pop_file_name(uint32_t key, node *root) {
    /* The root node shouldn't contain anything. */
    if (root->next == NULL)
        return NULL;

    const char *file_name = NULL;
    node *current = root->next, *last = root;
    for (;;) {
        if (current->key == key) {
            file_name = current->file_name;
            last->next = current->next;
            free(current->file_name);
            free(current);
            break;
        } else if (current->next == NULL) {
            break;
        } else {
            last = current;
            current = current->next;
        }
    }
    return file_name;
}


/* Document */
void defer_node(node *new, node *root) {
    node *current = root;
    for (;;) {
        if (current->next == NULL) {
            current->next = new;
            break;
        } else {
            current = current->next;
        }
    }
}


void test_linked_list(node *root) {
    for (node *current = root; current != NULL; current = current->next) {
        printf("NODE\n----\n");
        printf("KEY: %" PRIx32 "\n", current->key);
        printf("FILE_NAME: %s\n\n", current->file_name);
    }
}


/* Document */
void free_node(node *base) {
    if (base->next != NULL) {
        free_node(base->next);
        return;
    }
    free(base);
}
