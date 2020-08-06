/* -*- mode: C -*-
 *
 *       File:         rec-parse-field-name-str.c
 *       Date:         Sat Nov 13 15:42:21 2010
 *
 *       GNU recutils - rec_parse_field_name_str unit tests.
 *
 */

/* Copyright (C) 2009-2015 Jose E. Marchesi */

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
#include <stdlib.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_parse_field_name_str_nominal
 * Unit: rec_parse_field_name_str
 * Description:
 * + Parse valid field names.
 */
START_TEST(rec_parse_field_name_str_nominal)
{
  char *fname;

  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  fail_if (strcmp (fname, "foo") != 0);
  free (fname);

  fname = rec_parse_field_name_str ("foo:");
  fail_if (fname == NULL);
  fail_if (strcmp (fname, "foo") != 0);
  free (fname);
}
END_TEST

/*-
 * Test: rec_parse_field_name_str_invalid
 * Unit: rec_parse_field_name_str
 * Description:
 * + Try to parse invalid field names.
 */
START_TEST(rec_parse_field_name_str_invalid)
{
  char *fname;

  fname = rec_parse_field_name_str ("");
  fail_if (fname != NULL);

  fname = rec_parse_field_name_str ("fo!o");
  fail_if (fname != NULL);

  fname = rec_parse_field_name_str ("foo::");
  fail_if (fname != NULL);

  fname = rec_parse_field_name_str (":foo");
  fail_if (fname != NULL);

  fname = rec_parse_field_name_str ("foobar baz");
  fail_if (fname != NULL);

  fname = rec_parse_field_name_str ("foo:bar:baz");
  fail_if (fname != NULL);

  fname = rec_parse_field_name_str ("foo:baz!!#");
  fail_if (fname != NULL);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parse_field_name_str (void)
{
  TCase *tc = tcase_create ("rec_parse_field_name_str");
  tcase_add_test (tc, rec_parse_field_name_str_nominal);
  tcase_add_test (tc, rec_parse_field_name_str_invalid);
  
  return tc;
}

/* rec-parse-field-name-str.c */
