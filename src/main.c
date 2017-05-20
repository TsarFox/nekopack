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

#include <errno.h>
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

#define NEKOPACK_VERSION "2.1.0b1"


static void print_usage(char *progn) {
    fprintf(stderr, "Usage: %s [OPTIONS] (ARCHIVES) [PATHS]\n", progn);
}


static void print_version(void) {
    printf("Nekopack version %s\nProgrammed by Jakob. "
           "<http://jakob.space>\n", NEKOPACK_VERSION);
}


static void print_help(void) {
    printf("A tool for decompressing the XP3 archives used by Nekopara.\n\n"
           "   -h, --help\t\tDisplay this help page and exit.\n"
           "   -v, --version\tDisplay the currently installed version and "
           "exit.\n\n"
           "   -e, --extract\tExtract the contents of the archive. This is "
           "the default\n\t\t\taction if no mode argument is provided.\n"
           "   -l, --list\t\tList the contents of the archive.\n"
           "   -c, --create\t\tCreate a new XP3 archive.\n\n"
           "   -o, --output\t\tPath to extract files to.\n"
           "   -g, --game\t\tGame the archive is from. Required for file "
           "decryption.\n"
           "   -q, --quiet\t\tDon't display information about extracted "
           "files.\n\n"
           "Supported games: nekopara_volume_0, nekopara_volume_0_steam,\n"
           "nekopara_volume_1, nekopara_volume_1_steam\n");
}


static struct stream *load_table(struct stream *s) {
    uint8_t  compressed;
    uint64_t len, decompressed_len;
    stream_read(&compressed, s, sizeof(uint8_t));
    stream_read(&len, s, sizeof(uint64_t));
    stream_read(&decompressed_len, s, sizeof(uint64_t));
    if (compressed)
        return stream_inflate(s, len, decompressed_len);
    return stream_clone(s, len);
}


/* Creates any directories that do not already exist in `path`,
   including non-existent parents. */
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


/* Joins `name` and the output path specified in `p`. */
static char *get_path(struct params p, char *name) {
    size_t  name_len = strlen(name);
    char   *path     = malloc(p.out_len + name_len + 2);
    if (path == NULL)
        return NULL;

    strcpy(path, p.out);
    strcpy(path + p.out_len, name);
    return path;
}


static void list(struct table_entry *e) {
    if (e->filename == NULL || e->segments == NULL) {
        fprintf(stderr, "Unpaired entries found. Archive may be corrupted.\n");
        return;
    }
    printf("%s\n", e->filename);
}


static void extract(struct stream *s, struct table_entry *e, struct params p) {
    if (e->filename == NULL || e->segments == NULL) {
        fprintf(stderr, "Unpaired entries found. Archive may be corrupted.\n");
        return;
    }

    char *path = get_path(p, e->filename);
    if (path == NULL)
        return;

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
        uint8_t initial = derive_initial(k, e->key);
        uint8_t primary = derive_primary(k, e->key);
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
        if (errno == ENOENT) {
            perror(path);
        } else {
            fprintf(stderr, "Error allocating memory.\n");
        }
        return;
    }

    struct header *h = read_header(archive);
    if (h == NULL) {
        fprintf(stderr, "File is not an XP3 archive.\n");
        return;
    }
    stream_seek(archive, h->table_offset, SEEK_SET);

    struct stream *table = load_table(archive);
    if (table == NULL) {
        fprintf(stderr, "Error allocating memory.\n");
        return;
    }

    struct table_entry *root = read_table(table);
    if (root == NULL) {
        fprintf(stderr, "Error allocating memory.\n");
    }

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
    stream_free(archive);
    free(h);
}


/* Creates a new XP3 archive. The destination of the archive is the
   first string in `paths`, and the files at any following paths will be
   flattened into the archive. */
static void create_archive(char **paths, int argc, struct params p) {
    struct stream      *table_compressed, *file_compressed, *file;
    struct table_entry *cur;

    FILE *fp = fopen(paths[0], "wb+");
    if (fp == NULL) {
        perror(paths[0]);
        return;
    }

    struct table_entry *root = calloc(sizeof(struct table_entry), 1);
    if (root == NULL) {
        fprintf(stderr, "Error allocating memory.\n");
        return;
    }

    struct header *h = create_header();
    if (h == NULL) {
        fprintf(stderr, "Error allocating memory.\n");
        return;
    }

    struct stream *data  = stream_new(1);
    struct stream *table = stream_new(1);
    if (data == NULL || table == NULL) {
        fprintf(stderr, "Error allocating memory.\n");
        return;
    }

    uint64_t data_size  = 0;
    uint64_t table_size = 0;

    for (int i = 1; i < argc - p.vararg_index; i++) {
        file = stream_from_file(paths[i]);
        if (file == NULL) {
            perror(paths[i]);
            continue;
        }

        file_compressed = stream_deflate(file, file->len);
        if (file_compressed == NULL) {
            fprintf(stderr, "Unable to compress data.\n");
            return;
        }

        data_size += file_compressed->len;
        stream_concat(data, file_compressed, file_compressed->len);
        cur = add_file(root, paths[i]);
        if (cur == NULL) {
            fprintf(stderr, "Error allocating memory.\n");
            return;
        }

        /* TODO: Separate into own function. */
        table_size += 20 + strlen(cur->filename) * 2;
        table_size += 28 * cur->segment_count + 56;

        cur->segments = malloc(sizeof(struct segment *));
        if (cur->segments == NULL) {
            fprintf(stderr, "Error allocating memory.\n");
            return;
        }

        cur->segments[0] = malloc(sizeof(struct segment));
        if (cur->segments[0] == NULL) {
            fprintf(stderr, "Error allocating memory.\n");
            return;
        }

        cur->segments[0]->compressed        = 1;
        cur->segments[0]->offset            = 40 + data_size;
        cur->segments[0]->decompressed_size = file->len;
        cur->segments[0]->compressed_size   = file_compressed->len;

        stream_free(file_compressed);
        stream_free(file);
    }

    dump_table(table, root);

    stream_seek(table, 0, SEEK_SET);
    table_compressed = stream_deflate(table, table_size);
    stream_free(table);
    if (table_compressed == NULL) {
        fprintf(stderr, "Unable to compress data.\n");
        return;
    }

    h->table_size   = data->len; // This isn't right <:^)
    h->table_offset = 40 + data->len;
    dump_header(fp, h);

    /* FIXME: Formatting and this initial seek might not be necessary. */
    stream_seek(data, 0, SEEK_SET);
    stream_dump(fp, data, data->len);

    /* FIXME: May be an incompatible value. Also, move this shitty hack. */
    uint8_t compressed = 1;
    fwrite(&compressed,            sizeof(uint8_t),  1, fp);
    fwrite(&table_compressed->len, sizeof(uint64_t), 1, fp);
    fwrite(&table_size,            sizeof(uint64_t), 1, fp);

    stream_dump(fp, table_compressed, table_compressed->len);
}


int main(int argc, char **argv) {
    struct params p = parse_args(argc, argv);
    switch (p.mode) {
    case USAGE:
        print_usage(argv[0]);
        free(p.out);
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
        break;
    case CREATE:
        create_archive(argv + p.vararg_index, argc, p);
    }
    free(p.out);
    return EXIT_SUCCESS;
}
