/* -*- mode: C -*-
 *
 *       File:         rec-parser-perror.c
 *       Date:         Sat Nov 13 22:37:26 2010
 *
 *       GNU recutils - rec_parser_perror unit tests.
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

#include <check.h>
#include <rec.h>

/*-
 * Test: rec_parser_perror_nominal
 * Unit: rec_parser_perror
 * Description:
 * + Print the error message associated with a failed
 * + parsing.
 */
START_TEST(rec_parser_perror_nominal)
{
  rec_parser_t parser;
  rec_field_t field;
  char *str;

  str = "invalid field";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parser_error (parser));
  fail_if (rec_parse_field (parser, &field));
  fail_if (!rec_parser_error (parser));
  rec_parser_perror (parser, "expected error while parsing: ");
  rec_parser_destroy (parser);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parser_perror (void)
{
  TCase *tc = tcase_create ("rec_parser_perror");
  tcase_add_test (tc, rec_parser_perror_nominal);

  return tc;
}

/* End of rec-parser-perror.c */
