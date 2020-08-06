/* -*- mode: C -*-
 *
 *       File:         rec-parser-new-str.c
 *       Date:         Sat Nov 13 15:23:03 2010
 *
 *       GNU recutils - rec_parser_new_str unit tests.
 *
 */

/* Copyright (C) 2010-2015 Jose E. Marchesi */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <string.h>
#include <stdio.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_parser_new_str_nominal
 * Unit: rec_parser_new_str
 * Description:
 * + Create a rec parser.
 */
START_TEST(rec_parser_new_str_nominal)
{
  rec_parser_t parser;
  char *buffer = "foo: bar";

  parser = rec_parser_new_str (buffer, "buffer");
  fail_if (parser == NULL);

  rec_parser_destroy (parser);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parser_new_str (void)
{
  TCase *tc = tcase_create ("rec_parser_new_str");
  tcase_add_test (tc, rec_parser_new_str_nominal);

  return tc;
}

/* End of rec-parser-new-str.c */
