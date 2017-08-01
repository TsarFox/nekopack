/* test_io.c -- MinUnit test runner.

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

#include <stdio.h>

#include "minunit.h"
#include "test_cli.h"
#include "test_compress.h"
#include "test_crypto.h"
#include "test_encoding.h"
#include "test_header.h"
#include "test_io.h"
#include "test_table.h"

#define ANSI_RED   "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_BLUE  "\033[34m"
#define ANSI_BLINK "\033[5m"
#define ANSI_END   "\033[0m"

int tests_run = 0;


static const char *all_tests(void) {
    mu_run_test(test_out_path);
    mu_run_test(test_vararg_index);
    mu_run_test(test_game_id);
    mu_run_test(test_compress);
    mu_run_test(test_decompress);
    mu_run_test(test_derive_initial);
    mu_run_test(test_derive_primary);
    mu_run_test(test_stream_obj);
    mu_run_test(test_stream_rw);
    mu_run_test(test_stream_dump);
    mu_run_test(test_stream_realloc);
    mu_run_test(test_stream_xor);
    mu_run_test(test_stream_nav);
    mu_run_test(test_header_read);
    mu_run_test(test_header_creation);
    mu_run_test(test_table_list);
    mu_run_test(test_table_elif);
    mu_run_test(test_table_file);
    mu_run_test(test_decode_utf16le);
    mu_run_test(test_encode_utf16le);
    return NULL;
}


int main(void) {
    const char *res;

    printf("Test suite started.\n\n");
    res = all_tests();

    if (res != NULL) {
        printf(ANSI_RED "Test failed! " ANSI_END "\"%s\"\n", res);
        return 1;
    }

    printf(ANSI_GREEN "Test suite finished successfully.\n" ANSI_END);
    printf(ANSI_BLUE "%d tests were run.\n" ANSI_END, tests_run);
    return 0;
}
