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
#include <string.h>

#include "decompress.h"
#include "extract.h"
#include "file.h"
#include "write.h"


/* Finds an eliF entry with the given key in a linked list and returns
   the associated filename, removing it from the linked list. */
char *pop_file_name(uint32_t key, elif_node *root) {
    char *file_name = NULL;
    elif_node *current = root->next, *last = root;
    for (;;) {
        if (current->key == key) {
            file_name = current->file_name;
            last->next = current->next;
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


/* Iterates through the linked list of file entries and writes every
   entry to disk, according to information specified by the node. */
void write_files(file_node *file_root, elif_node *elif_root, Bytef *start) {
    FILE *output;
    file_node *current;
    Bytef *compressed_buffer, *decompressed_buffer;
    for (current = file_root->next; current != NULL; current = current->next) {
        char *file_name = pop_file_name(current->key, elif_root);
        if (file_name == NULL) {
            fprintf(stderr, "File found without matching eliF entry.\n");
            continue;
        }
        compressed_buffer = malloc(current->compressed_size);
        for (int i = 0; i < current->segment_count; i++)
            free(current->segments[i]);
        free(current->segments);
        /* memcpy(compressed_buffer, start + current->offset, */
        /*        current->decompressed_size); */
        /* if (current->compressed) { */
        /*     fprintf(stderr, "Not implemented :^)\n"); // don't leave this in lol */
        /*     continue; */
        /* } else { */
        /*     output = fopen(file_name, "wb+"); */
        /*     fwrite(compressed_buffer, current->compressed_size, 1, output); */
        /*     fclose(output); */
        /* } */

        free(compressed_buffer);
        break;
    }
}


/* Inserts an eliF entry at the end of a linked list. */
void defer_elif_node(elif_node *new, elif_node *root) {
    elif_node *current = root;
    for (;;) {
        if (current->next == NULL) {
            current->next = new;
            break;
        } else {
            current = current->next;
        }
    }
}


/* Inserts a File entry node at the end of a linked list. */
void defer_file_node(file_node *new, file_node *root) {
    file_node *current = root;
    for (;;) {
        if (current->next == NULL) {
            current->next = new;
            break;
        } else {
            current = current->next;
        }
    }
}


/* Iterates through the linked list and frees all entries. */
void free_elif_nodes(elif_node *base) {
    if (base->next != NULL)
        free_elif_nodes(base->next);
    free(base->file_name);
    free(base);
}


/* Iterates through the linked list and frees all entries. */
void free_file_nodes(file_node *base) {
    if (base->next != NULL)
        free_file_nodes(base->next);
    free(base);
}


void test_elif_linked_list(elif_node *root) {
    for (elif_node *current = root; current != NULL; current = current->next) {
        printf("ELIF NODE\n---------\n");
        printf("KEY: %" PRIx32 "\n", current->key);
        printf("FILE_NAME: %s\n\n", current->file_name);
    }
}


void test_file_linked_list(file_node *root) {
    for (file_node *current = root; current != NULL; current = current->next) {
        printf("FILE NODE\n---------\n");
        printf("KEY: %" PRIx32 "\n\n", current->key);
    }
}
