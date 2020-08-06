/* -*- mode: C -*-
 *
 *       File:         rec-parser-eof.c
 *       Date:         Sat Nov 13 22:21:31 2010
 *
 *       GNU recutils - rec_parser_eof unit tests.
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
 * Test: rec_parser_eof_nominal
 * Unit: rec_parser_eof
 * Description:
 * + Check for EOF in a parser operating a stream.
 */
START_TEST(rec_parser_eof_nominal)
{
  rec_parser_t parser;
  rec_field_t field;
  char *str;

  str = "foo: bar";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (rec_parser_eof (parser));
  fail_if (!rec_parse_field (parser, &field));
  fail_if (!rec_parser_eof (parser));
  rec_field_destroy (field);
  rec_parser_destroy (parser);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parser_eof (void)
{
  TCase *tc = tcase_create ("rec_parser_eof");
  tcase_add_test (tc, rec_parser_eof_nominal);

  return tc;
}

/* End of rec-parser-eof.c */
