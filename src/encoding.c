/* encoding.c -- Code for working with encoded strings.

   Copyright (C) 2017 Jakob Tsar-Fox, All Rights Reserved.

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


/* Wrapper for iconv, using the conversion specified by `conv`. */
static void convert(char *in_buf, char *out_buf, size_t len, iconv_t conv) {
    size_t in_size = len, out_size = len;
    char *in_start = in_buf, *out_start = out_buf;
    iconv(conv, &in_start, &in_size, &out_start, &out_size);
}


/* Decodes the UTF-16LE string specified by `in_buf` into `out_buf`. */
void utf16le_decode(char *in_buf, char *out_buf, size_t len) {
    iconv_t conv = iconv_open("UTF-8", "UTF-16LE");
    convert(in_buf, out_buf, len, conv);
    iconv_close(conv);
}
