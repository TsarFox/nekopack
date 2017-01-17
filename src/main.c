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

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "decompress.h"
#include "extract.h"

#define XP3_MAGIC "XP3\x0d\x0a\x20\x0a\x1a\x8b\x67\x01"
#define XP3_TABLE_OFFSET 11
#define XP3_VERSION_OFFSET 19

int is_xp3_archive(FILE *archive);
int get_archive_version(FILE *archive);
uint64_t get_table_offset(FILE *archive, uint8_t archive_version);

/* Global instance of the command-line configuration structure. */
struct configuration arguments;


int main(int argc, char *argv[]) {
    arguments = parse_args(argc, argv);

    FILE *archive = fopen(arguments.archive, "rb");
    if (archive == NULL) {
        /* Including the expanded path in the error message is
           much more useful to the user than `perror("fopen")`. */
        perror(arguments.archive);
        exit(EXIT_FAILURE);
    } else if (!is_xp3_archive(archive)) {
        fprintf(stderr, "%s is not an XP3 archive.\n", arguments.archive);
        fclose(archive);
        exit(EXIT_FAILURE);
    }

    int archive_version = get_archive_version(archive);
    uint64_t table_offset = get_table_offset(archive, archive_version);

    uint8_t compressed;
    uint64_t compressed_size, decompressed_size;
    fseek(archive, table_offset, SEEK_SET);
    fread(&compressed, sizeof(uint8_t), 1, archive);
    fread(&compressed_size, sizeof(uint64_t), 1, archive);
    fread(&decompressed_size, sizeof(uint64_t), 1, archive);

    /* Every task that hasn't already been handled needs a
       decompressed instance of the archive table. Decompression
       done in memory because it's $CURRENT_YEAR. */
    memory_stream data_stream;
    memory_stream compressed_data = read_to_stream(archive, compressed_size);
    if (compressed) {
        data_stream = decompress_stream(compressed_data, decompressed_size);
        free(compressed_data.start);
    } else {
        data_stream = compressed_data;
    }

    switch (arguments.mode) {
        case EXTRACT:
            extract(data_stream, archive);
            break;
        case LIST:
            list(data_stream);
    }

    free(data_stream.start);
    fclose(archive);
    return 0;
}


/* Check of the file's magic number to decide
   whether or not the file a valid XP3 archive. */
int is_xp3_archive(FILE *archive) {
    int ret = 1;
    char* buffer = malloc(11);
    rewind(archive);
    fread(buffer, 11, 1, archive);
    if (memcmp(buffer, XP3_MAGIC, 11))
        ret = 0;
    free(buffer);
    return ret;
}


/* Returns the version of XP3 used to pack the archive. */
int get_archive_version(FILE *archive) {
    uint32_t version;
    fseek(archive, XP3_VERSION_OFFSET, SEEK_SET);
    fread(&version, sizeof(uint32_t), 1, archive);
    /* 0x00 indicates version 1, and 0x01 indicates version 2. */
    return version == 1 ? 2 : 1;
}


/* Subroutine for finding the archive's table offset. If
   the minor_version is invalid, the program will exit.  */
uint64_t get_table_offset(FILE *archive, uint8_t archive_version) {
    uint64_t table_offset;
    fseek(archive, XP3_TABLE_OFFSET, SEEK_SET);
    fread(&table_offset, sizeof(uint64_t), 1, archive);
    if (archive_version == 1)
        return table_offset;

    /* The minor version is only present in XP3 version 2. */
    uint32_t minor_version;
    fread(&minor_version, sizeof(uint32_t), 1, archive);
    if (minor_version != 1) {
        fprintf(stderr, "Archive version not implemented.\n");
        fclose(archive);
        exit(EXIT_FAILURE);
    }

    /* The value initally read as table_offset is actually an offset to
       the real table offset. (Try saying that five times fast.) */
    fseek(archive, table_offset, SEEK_SET);
    /* Table flags and size are ignored in the parsing process,
       but are read anyway to advance the FILE pointer. */
    fseek(archive, sizeof(uint8_t) + sizeof(uint64_t), SEEK_CUR);
    fread(&table_offset, sizeof(uint64_t), 1, archive);
    return table_offset;
}
