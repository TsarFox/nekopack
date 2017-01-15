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

#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "crypto.h"
#include "decompress.h"
#include "extract.h"
#include "file.h"
#include "write.h"

/* Global instance of the command-line configuration structure. */
extern struct configuration arguments;


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


/* Creates all of the paths in a filename. */
void make_paths(char *file_name) {
    /* Max filename size. */
    char *buffer = calloc(0x100, 1);
    char *buffer_start = buffer;
    for (int i = 0; i < 0x100 && file_name[i] != '\0'; i++) {
        *buffer++ = file_name[i];
        if (file_name[i] == '/') {
            *buffer = '\0';
            printf("Creating directory %s\n", buffer_start);
            mkdir(buffer_start, 0777);
        }
    }
    free(buffer_start);
}


/* Iterates through the linked list of file entries and writes every
   entry to disk, according to information specified by the node. */
void write_files(file_node *file_root, elif_node *elif_root, FILE *archive) {
    file_node *current;
    key encryption_key = get_encryption_key(arguments.game);
    Bytef *compressed_buffer, *decompressed_buffer, *out_buffer, *out_start;
    for (current = file_root->next; current != NULL; current = current->next) {
        char *file_name = pop_file_name(current->key, elif_root);
        if (file_name == NULL)
            continue;

        if (!arguments.quiet) {
            printf("Inflating %s (%" PRIu64 " bytes)\n",
                   file_name, current->file_size);
        }

        out_buffer = malloc(current->file_size);
        if (out_buffer == NULL) {
            fprintf(stderr, "Insufficient memory to decompress.\n");
            return;
        }

        /* out_start is kept so out_buffer can be incremented. */
        out_start = out_buffer;
        for (uint64_t i = 0; i < current->segment_count; i++) {
            segment *chunk = current->segments[i];
            fseek(archive, chunk->offset, SEEK_SET);
            compressed_buffer = malloc(chunk->compressed_size);
            fread(compressed_buffer, chunk->compressed_size, 1, archive);

            if (chunk->compressed) {
                decompressed_buffer = inflate_chunk(compressed_buffer,
                                                    chunk->compressed_size,
                                                    chunk->decompressed_size);
                free(compressed_buffer);
            } else {
                decompressed_buffer = compressed_buffer;
            }
            memcpy(out_buffer, decompressed_buffer, chunk->decompressed_size);
            out_buffer += chunk->decompressed_size;
            free(decompressed_buffer);
        }

        if (arguments.game != NO_CRYPTO) {
            decrypt_buffer(out_start, current->file_size,
                           encryption_key, current->key);
        }

        make_paths(file_name);
        FILE *output = fopen(file_name, "wb+");
        if (output == NULL) {
            perror(file_name);
            break;
        }
        fwrite(out_start, current->file_size, 1, output);
        fclose(output);
        free(file_name);
        free(out_start);
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
    if (base->next != NULL) {
        free_elif_nodes(base->next);
    }
    free(base->file_name);
    free(base);
}


/* Iterates through the linked list and frees all entries. */
void free_file_nodes(file_node *base) {
    if (base->next != NULL)
        free_file_nodes(base->next);
    free(base);
}
