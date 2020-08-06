/* -*- mode: C -*-
 *
 *       File:         rec-parse-record-str.c
 *       Date:         Sat Nov 13 20:16:11 2010
 *
 *       GNU recutils - rec_parse_record_str unit tests.
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
 * Test: rec_parse_record_str_nominal
 * Unit: rec_parse_record_str
 * Description:
 * + Parse valid records from strings.
 */
START_TEST(rec_parse_record_str_nominal)
{
  rec_record_t record;
  rec_field_t field;
  char *fname;

  record = rec_parse_record_str ("foo: bar");
  fail_if (record == NULL);
  fname = rec_parse_field_name_str ("foo");
  fail_if (fname == NULL);
  field = (rec_field_t) rec_mset_get_at (rec_record_mset (record), MSET_FIELD, 0);
  fail_if (strcmp (rec_field_value (field), "bar") != 0);
  fail_if (!rec_field_name_equal_p (fname,
                                    rec_field_name (field)));
  free (fname);
  rec_record_destroy (record);
}
END_TEST

/*-
 * Test: rec_parse_record_str_invalid
 * Unit: rec_parse_record_str
 * Description:
 * + Try to parse invalid records from strings.
 */
START_TEST(rec_parse_record_str_invalid)
{
  rec_record_t record;
  
  record = rec_parse_record_str ("");
  fail_if (record != NULL);

  record = rec_parse_record_str (" ");
  fail_if (record != NULL);
}
END_TEST

/*
 * Test creation function.
 */
TCase *
test_rec_parse_record_str (void)
{
  TCase *tc = tcase_create ("rec_parse_record_str");
  tcase_add_test (tc, rec_parse_record_str_nominal);
  tcase_add_test (tc, rec_parse_record_str_invalid);
  
  return tc;
}

/* End of rec-parse-record-str.c */
