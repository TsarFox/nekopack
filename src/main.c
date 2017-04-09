/* main.c -- Command-line entry point to the Nekopack.

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

#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "compress.h"
#include "crypto.h"
#include "header.h"
#include "io.h"
#include "table.h"

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define VERSION_STR "2.0.0"


/* Writes usage information to stderr. */
static void print_usage(char *prog_name) {
    fprintf(stderr, "Usage: %s [OPTIONS] (ARCHIVES) [PATHS]\n", prog_name);
}


/* Writes versioning information to stdout. */
static void print_version(void) {
    printf("Nekopack version %s\nProgrammed by Jakob. "
           "<http://jakob.space>\n", VERSION_STR);
}


/* Writes help information to stdout. */
static void print_help(void) {
    printf("A tool for decompressing the XP3 archives used by Nekopara.\n\n"
           "   -h, --help\t\tDisplay this help page and exit.\n"
           "   -v, --version\tDisplay the currently installed version and "
           "exit.\n\n"
           "   -e, --extract\tExtract the contents of the archive. This is "
           "the default\n\t\t\taction if no mode argument is provided.\n"
           "   -l, --list\t\tList the contents of the archive.\n\n"
           "   -o, --output\t\tPath to extract files to.\n"
           "   -g, --game\t\tGame the archive is from. Required for file "
           "decryption.\n"
           "   -q, --quiet\t\tDon't display information about extracted "
           "files.\n\n"
           "Supported games: nekopara_volume_0, nekopara_volume_0_steam,\n"
           "nekopara_volume_1, nekopara_volume_1_steam\n");
}


/* Inflates the table according to information in the header. */
static struct stream *load_table(struct stream *s) {
    uint8_t compressed;
    uint64_t len, decompressed_len;
    stream_read(&compressed, s, sizeof(uint8_t));
    stream_read(&len, s, sizeof(uint64_t));
    stream_read(&decompressed_len, s, sizeof(uint64_t));
    if (compressed)
        return stream_inflate(s, len, decompressed_len);
    return stream_clone(s, len);
}


/* Creates any directories that do not already exist in `path`. */
void make_dirs(char *path) {
    char *buf       = calloc(0x100, 1);
    char *buf_start = buf;
    struct stat tmp = {0};
    for (int i = 0; i < 0x100 && path[i] != '\0'; i++) {
        if (path[i] == '/') {
            *buf++ = '/';
            *buf   = '\0';
            if (stat(buf_start, &tmp) == -1)
                mkdir(buf_start, 0777);
        } else {
            *buf++ = path[i];
        }
    }
    free(buf_start);
}


/* Concatenates `name` and the output path specified in `p`. */
static char *get_path(struct params p, char *name) {
    size_t name_len = strlen(name);
    char *path = malloc(p.out_len + name_len + 2);
    strcpy(path, p.out);
    strcpy(path + p.out_len, name);
    return path;
}


/* Prints the filename of the entry specified by `e` to stdout. */
static void list(struct table_entry *e) {
    if (e->filename == NULL || e->segments == NULL) {
        fprintf(stderr, "Inconsistency detected. Archive may be corrupted.\n");
        return;
    }
    printf("%s\n", e->filename);
}


/* Extracts the archive entry specified by `e` to disk. */
static void extract(struct stream *s, struct table_entry *e, struct params p) {
    if (e->filename == NULL || e->segments == NULL) {
        fprintf(stderr, "Inconsistency detected. Archive may be corrupted.\n");
        return;
    }

    char *path = get_path(p, e->filename);
    make_dirs(path);

    FILE           *fp        = fopen(path, "wb+");
    struct stream  *segm_data;
    struct segment *segm;

    if (fp == NULL) {
        perror(path);
        exit(EXIT_FAILURE);
    } else if (p.verbose) {
        printf("Extracting %s to %s...\n", e->filename, path);
    }

    for (uint64_t i = 0; i < e->segment_count; i++) {
        segm = e->segments[i];
        stream_seek(s, segm->offset, SEEK_SET);
        if (e->segments[i]->compressed) {
            segm_data = stream_inflate(s, segm->compressed_size,
                                       segm->decompressed_size);
        } else {
            segm_data = stream_clone(s, segm->compressed_size);
        }

        struct game_key k = get_key(p.game);
        uint8_t initial   = derive_initial(k, e->key);
        uint8_t primary   = derive_primary(k, e->key);
        stream_xor(segm_data, initial, primary);

        stream_dump(fp, segm_data, segm_data->len);
        stream_free(segm_data);
    }

    fclose(fp);
    free(path);
}


/* Loads an XP3 archive and maps the function associated to the current
   mode of operation to every entry. */
static void map_entries(char *path, struct params p) {
    struct stream *archive = stream_from_file(path);
    if (archive == NULL) {
        perror(path);
        return;
    }

    struct header *h = read_header(archive);
    stream_seek(archive, h->table_offset, SEEK_SET);

    struct stream      *table = load_table(archive);
    struct table_entry *root  = read_table(table);

    for (struct table_entry *cur = root->next; cur != NULL; cur = cur->next) {
        switch (p.mode) {
        case LIST:
            list(cur);
            break;
        case EXTRACT:
            extract(archive, cur, p);
        }
    }

    entry_free(root);
    stream_free(table);
    free(h);
    stream_free(archive);
}


int main(int argc, char **argv) {
    struct params p = parse_args(argc, argv);
    switch (p.mode) {
    case USAGE:
        print_usage(argv[0]);
        params_free(p);
        return EXIT_FAILURE;
    case VERSION:
        print_version();
        break;
    case HELP:
        print_help();
        break;
    case LIST:
    case EXTRACT:
        for (int i = p.vararg_index; i < argc; i++)
            map_entries(argv[i], p);
    }
    params_free(p);
    return EXIT_SUCCESS;
}
