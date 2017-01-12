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

/* Because File entries won't necessarily follow the associated
   eliF entry, filenames and hashes are stored in a linked list. */
typedef struct elif_node {
    uint32_t key; /* Key associated with matching File entry. */
    char *file_name; /* Name of file. */
    struct elif_node *next; /* Pointer to the next node. */
} elif_node;

/* Iterates through the linked list of file entries and writes every
   entry to disk, according to information specified by the node. */
void write_files(file_node *file_root, elif_node *elif_root, FILE *archive);

/* Inserts an eliF entry at the end of a linked list. */
void defer_elif_node(elif_node *new, elif_node *root);

/* Inserts a File entry node at the end of a linked list. */
void defer_file_node(file_node *new, file_node *root);

/* Iterates through the linked list and frees all entries. */
void free_elif_nodes(elif_node *base);

/* Iterates through the linked list and frees all entries. */
void free_file_nodes(file_node *base);
