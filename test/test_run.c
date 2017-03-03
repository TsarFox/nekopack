/* test_io.c -- MinUnit test runner.

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

#include <stdio.h>

#include "minunit.h"

#include "test_cli.h"
#include "test_encoding.h"
#include "test_header.h"
#include "test_io.h"
#include "test_table.h"

int tests_run = 0;


static char *run_all_tests(void) {
    mu_run_test(test_out_path);
    mu_run_test(test_stream_obj);
    mu_run_test(test_stream_rw);
    mu_run_test(test_stream_dump);
    mu_run_test(test_stream_realloc);
    mu_run_test(test_stream_xor);
    mu_run_test(test_stream_nav);
    mu_run_test(test_header_read);
    mu_run_test(test_table_list);
    mu_run_test(test_table_elif);
    mu_run_test(test_table_file);
    mu_run_test(test_decode_utf16le);
    return NULL;
}


int main(void) {
    char *ret = run_all_tests();
    if (ret) {
        printf("%s\n", ret);
        return 1;
    }
    printf("Successful tests: %d\n", tests_run);
    return 0;
}
