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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#include "cli.h"

#define XP3_MAGIC "XP3\x0d\x0a\x20\x0a\x1a\x8b\x67\x01"
#define XP3_VERSION_1_TABLE_OFFSET 11
#define XP3_VERSION_OFFSET 19

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int is_xp3_archive(FILE *archive);
int get_archive_version(FILE *archive);
uint8_t table_is_compressed(FILE *archive, uint8_t table_offset);
uint64_t get_table_offset(FILE *archive, uint8_t archive_version);


int main(int argc, char *argv[]) {
    struct configuration arguments = parse_args(argc, argv);

    FILE *archive = fopen(argv[1], "rb");
    if (!is_xp3_archive(archive)) {
        fprintf(stderr, "File is not an XP3 archive.\n");
        fclose(archive);
        exit(EXIT_FAILURE);
    }
    /* After the table_is_compressed byte is 8 bytes containing the
       compressed size followed by 8 bytes containing the original size
       (if the archive is compressed). Use zlib's inflate function to
       decompress the compressed size worth of chunks if it's compressed.

       The decompressed data will contain a four-byte magic number, 
       followed by 8 bytes containing the file size, followed by the
       file size worth of data.

       See the following link for magic numbers and entry handling
       procedures.

       https://github.com/vn-tools/arc_unpacker/blob/master/src/dec
       /kirikiri/xp3_archive_decoder.cc */
    fclose(archive);
    return 0;
}


/* Very simple check of the archive's magic number to decide
   whether or not it really is a valid XP3 archive. */
int is_xp3_archive(FILE *archive) {
    char magic_buffer[12] = {0};
    rewind(archive);
    fread(magic_buffer, sizeof(magic_buffer) - 1, 1, archive);
    if (strcmp(magic_buffer, XP3_MAGIC))
        return 0;
    return 1;
}


/* Returns the version of XP3 used to pack the archive. */
int get_archive_version(FILE *archive) {
    uint32_t version_word;
    fseek(archive, XP3_VERSION_OFFSET, SEEK_SET);
    fread(&version_word, sizeof(uint32_t), 1, archive);
    return version_word == 1 ? 2 : 1;
}


/* Subroutine for finding the archive's table offset. If the
   minor_version is invalid, the program will exit.  */
uint64_t get_table_offset(FILE *archive, uint8_t archive_version) {
    fseek(archive, XP3_VERSION_1_TABLE_OFFSET, SEEK_SET);
    if (archive_version == 1) {
        uint64_t table_offset;
        fread(&table_offset, sizeof(uint64_t), 1, archive);
        return table_offset;
    }
    uint64_t additional_header_offset, table_offset;
    uint32_t minor_version;
    fread(&additional_header_offset, sizeof(uint64_t), 1, archive);
    fread(&minor_version, sizeof(uint32_t), 1, archive);
    if (minor_version != 1) {
        fprintf(stderr, "Version not implemented.\n");
        exit(EXIT_FAILURE);
    }
    fseek(archive, additional_header_offset, SEEK_CUR);
    fseek(archive, sizeof(uint8_t) + sizeof(uint64_t), SEEK_CUR);
    fread(&table_offset, sizeof(uint64_t), 1, archive);
    return table_offset;
}


/* Function that returns the byte representing
   whether or not the archive was compressed. */
uint8_t table_is_compressed(FILE *archive, uint8_t table_offset) {
    uint8_t is_compressed;
    fseek(archive, table_offset, SEEK_SET);
    fread(&is_compressed, sizeof(uint8_t), 1, archive);
    return is_compressed;
}


void *inflate_table(FILE *archive, uint8_t table_offset) {
    return 0x0;
}
