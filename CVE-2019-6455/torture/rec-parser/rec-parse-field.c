/* -*- mode: C -*-
 *
 *       File:         rec-parse-field.c
 *       Date:         Sat Nov 13 17:37:11 2010
 *
 *       GNU recutils - rec_parse_field unit tests.
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
#include <stdlib.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_parse_field_nominal
 * Unit: rec_parse_field
 * Description:
 * + Parse valid fields.
 */
START_TEST(rec_parse_field_nominal)
{
  rec_parser_t parser;
  rec_field_t field;
  char *fname;
  char *str;

  str = "foo: bar";
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  fail_if (strcmp (rec_field_value (field), "bar") != 0);
  free (fname);
  rec_field_destroy (field);
  rec_parser_destroy (parser);

  str = "foo:  bar";
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  fail_if (strcmp (rec_field_value (field), " bar") != 0);
  free (fname);
  rec_field_destroy (field);
  rec_parser_destroy (parser);

  str = "foo: bar ";
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  fail_if (strcmp (rec_field_value (field), "bar ") != 0);
  free (fname);
  rec_field_destroy (field);
  rec_parser_destroy (parser);

  str = "foo:\n";
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  fail_if (strcmp (rec_field_value (field), "") != 0);
  free (fname);
  rec_field_destroy (field);
  rec_parser_destroy (parser);

  str = "foo:\n+ bar";
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  fail_if (strcmp (rec_field_value (field), "\nbar") != 0);
  free (fname);
  rec_field_destroy (field);
  rec_parser_destroy (parser);

  str = "foo:\n+bar";
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  fail_if (strcmp (rec_field_value (field), "\nbar") != 0);
  free (fname);
  rec_field_destroy (field);
  rec_parser_destroy (parser);

  str = "foo: bar\n+baz";
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  fail_if (strcmp (rec_field_value (field), "bar\nbaz") != 0);
  free (fname);
  rec_field_destroy (field);
  rec_parser_destroy (parser);

  str = "foo: one\n+\n+ \n+ two";
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  fail_if (strcmp (rec_field_value (field), "one\n\n\ntwo") != 0);
  free (fname);
  rec_field_destroy (field);
  rec_parser_destroy (parser);

  str = "foo: bar \\\nbaz";
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  fail_if (strcmp (rec_field_value (field), "bar baz") != 0);
  free (fname);
  rec_field_destroy (field);
  rec_parser_destroy (parser);

  /*  str = "foo:";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field (parser, &field));
  rec_parser_destroy (parser);
  fclose (stm); */
}
END_TEST

/*-
 * Test: rec_parse_field_invalid
 * Unit: rec_parse_field
 * Description:
 * + Try to parse invalid fields.
 */
START_TEST(rec_parse_field_invalid)
{
  rec_parser_t parser;
  rec_field_t field;
  char *str;

  str = " ";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field (parser, &field));
  rec_parser_destroy (parser);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parse_field (void)
{
  TCase *tc = tcase_create ("rec_parse_field");
  tcase_add_test (tc, rec_parse_field_nominal);
  tcase_add_test (tc, rec_parse_field_invalid);

  return tc;
}

/* End of rec-parse-field.c */
