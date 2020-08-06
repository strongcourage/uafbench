/* -*- mode: C -*-
 *
 *       File:         rec-write-field-name.c
 *       Date:         Sun Nov 14 11:32:17 2010
 *
 *       GNU recutils - rec_write_field_name unit tests.
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
 * Test: rec_write_field_name_nominal
 * Unit: rec_write_field_name
 * Description:
 * + Write field names.
 */
START_TEST(rec_write_field_name_nominal)
{
  rec_writer_t writer;
  char *fname;
  char *str;
  size_t str_size;

  fname = "foo";
  writer = rec_writer_new_str (&str, &str_size);
  rec_writer_set_mode (writer, REC_WRITER_NORMAL);
  fail_if (!rec_write_field_name (writer, fname));
  rec_writer_destroy (writer);
  fail_if (strcmp (str, "foo:") != 0);
  free (str);
}
END_TEST

/*-
 * Test: rec_write_field_name_sexp
 * Unit: rec_write_field_name
 * Description:
 * + Write field names.
 */
START_TEST(rec_write_field_name_sexp)
{
  rec_writer_t writer;
  char *fname;
  char *str;
  size_t str_size;

  fname = "foo";
  writer = rec_writer_new_str (&str, &str_size);
  rec_writer_set_mode (writer, REC_WRITER_SEXP);
  fail_if (!rec_write_field_name (writer, fname));
  rec_writer_destroy (writer);
  fail_if (strcmp (str, "\"foo\"") != 0);
  free (str);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_write_field_name (void)
{
  TCase *tc = tcase_create ("rec_write_field_name");
  tcase_add_test (tc, rec_write_field_name_nominal);
  tcase_add_test (tc, rec_write_field_name_sexp);

  return tc;
}

/* End of rec-write-field-name.c */
