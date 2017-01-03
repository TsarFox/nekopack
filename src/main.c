/* main.c

   The entry point to Nekopack.

   Copyright (C) 2013 Jakob Tsar-Fox, All Rights Reserved.

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


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XP3_MAGIC "XP3\x0d\x0a\x20\x0a\x1a\x8b\x67\x01"
#define XP3_VERSION_1_TABLE_OFFSET 11
#define XP3_VERSION_OFFSET 19

int is_xp3_archive(FILE *archive);
int get_archive_version(FILE *archive);
int get_table_offset(FILE *archive, uint8_t archive_version);
int table_is_compressed(FILE *archive, uint8_t table_offset);


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s (PATH TO ARCHIVE)\n", argv[0]);
        exit(1);
    }
    uint8_t archive_version;
    FILE *archive = fopen(argv[1], "rb");
    if (!is_xp3_archive(archive)) {
        fprintf(stderr, "File is not an XP3 archive.\n");
        fclose(archive);
        exit(1);
    }
    archive_version = get_archive_version(archive);
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


/* Returns whether or not the stream represents an XP3 archive. */
int is_xp3_archive(FILE *archive) {
    char *magic_buffer = malloc(12);
    if (magic_buffer == NULL) {
        perror("calloc");
        fclose(archive);
        exit(1);
    }
    rewind(archive);
    fread(magic_buffer, 11, 1, archive);
    if (strcmp(magic_buffer, XP3_MAGIC)) {
        printf("Not an XP3 archive\n");
        return 0;
    }
    return 1;
}


/* Returns the version of (???) used to pack the archive. */
int get_archive_version(FILE *archive) {
    uint8_t archive_version;
    fseek(archive, XP3_VERSION_OFFSET, SEEK_SET);
    fread(&archive_version, 1, 1, archive);
    return archive_version;
}


/* Returns the table offset for the archive. */
int get_table_offset(FILE *archive, uint8_t archive_version) {
    if (archive_version == 1) {
        uint8_t table_offset;
        fseek(archive, XP3_VERSION_1_TABLE_OFFSET, SEEK_SET);
        fread(&table_offset, 1, 1, archive);
        return table_offset;
    }
    fprintf(stderr, "Not implemented.\n");
    exit(1);
}


/* Returns whether or not the archive is compressed. */
int table_is_compressed(FILE *archive, uint8_t table_offset) {
    uint8_t is_compressed;
    fseek(archive, table_offset, SEEK_SET);
    fread(&is_compressed, 1, 1, archive);
    return is_compressed;
}
