/* encoding.c -- Code for working with encoded strings.

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

#include <iconv.h>
#include <string.h>


/* Wrapper for iconv, using the conversion specified by `conv`. */
static void convert(char *in_buf, char *out_buf, size_t in_len,
                    size_t out_len, iconv_t conv) {
    size_t  in_size  = in_len,  out_size  = out_len;
    char   *in_start = in_buf, *out_start = out_buf;
    iconv(conv, &in_start, &in_size, &out_start, &out_size);
}


/* Decodes the UTF-16LE string specified by `in_buf` into `out_buf`. */
void utf16le_decode(char *in_buf, char *out_buf, size_t len) {
    iconv_t conv = iconv_open("UTF-8", "UTF-16LE");
    convert(in_buf, out_buf, len, len, conv);
    iconv_close(conv);
}


/* Encodes the UTF-8 string specified by `in_buf` into a UTF-16LE string
   stored in `out_buf`. */
void utf16le_encode(char *in_buf, char *out_buf, size_t len) {
    iconv_t conv = iconv_open("UTF-16LE", "UTF-8");
    /* This is done to ensure that the null-terminator is included in
       the encoded string. */
    if (strlen(in_buf) == len)
        len++;
    convert(in_buf, out_buf, len, len * 2 + 2, conv);
    iconv_close(conv);
}
