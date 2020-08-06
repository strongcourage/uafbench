/* -*- mode: C -*-
 *
 *       File:         rec-parse-field-name.c
 *       Date:         Sat Nov 13 16:31:54 2010
 *
 *       GNU recutils - rec_parse_field_name unit tests.
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
 * Test: rec_parse_field_name_nominal
 * Unit: rec_parse_field_name
 * Description:
 * + Parse valid field names.
 */
START_TEST(rec_parse_field_name_nominal)
{
  rec_parser_t parser;
  char *fname;
  char *str;

  str = "foo:";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field_name (parser, &fname));
  fail_if (strcmp (fname, "foo") != 0);
  free (fname);
  rec_parser_destroy (parser);

  str = "%foo:";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field_name (parser, &fname));
  fail_if (strcmp (fname, "%foo") != 0);
  free (fname);
  rec_parser_destroy (parser);
}
END_TEST

/*-
 * Test: rec_parse_field_name_invalid
 * Unit: rec_parse_field_name
 * Description:
 * + Try to parse invalid field names.
 */
START_TEST(rec_parse_field_name_invalid)
{
  rec_parser_t parser;
  char *fname;
  char *str;  

  str = " ";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);

  str = "foo";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);

  str = ":foo";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);

  str = "fo!o";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);

  str = "%%foo";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parse_field_name (void)
{
  TCase *tc = tcase_create ("rec_parse_field_name");
  tcase_add_test (tc, rec_parse_field_name_nominal);
  tcase_add_test (tc, rec_parse_field_name_invalid);

  return tc;
}

/* End of rec-parse-field-name.c */
