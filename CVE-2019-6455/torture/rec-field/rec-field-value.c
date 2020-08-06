/* -*- mode: C -*-
 *
 *       File:         rec-field-value.c
 *       Date:         Sun Mar  1 17:04:00 2009
 *
 *       GNU recutils - rec_field_value unit tests
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
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_field_value_empty
 * Unit: rec_field_value
 * Description:
 * + Get the value of a field with an empy value
 * +
 * + 1. The call should not produce an error.
 * + 2. The value of the field should be properly
 * +    returned.
 */
START_TEST(rec_field_value_empty)
{
  rec_field_t field;
  const char *fname;
  const char *field_value;
  
  fname = "";
  field = rec_field_new (fname, "");
  fail_if(field == NULL);

  field_value = rec_field_value (field);
  fail_if(strcmp (field_value, "") != 0);

  rec_field_destroy (field);
}
END_TEST

/*-
 * Test: rec_field_value_nonempty
 * Unit: rec_field_value
 * Description:
 * + Get the value of a field with a non-empty value
 * +
 * + 1. The call should not produce an error.
 * + 2. The value of the field should be properly
 * +    returned.
 */
START_TEST(rec_field_value_nonempty)
{
  rec_field_t field;
  const char *fname;
  const char *field_value;
  
  fname = "";
  field = rec_field_new (fname, "foo");
  fail_if(field == NULL);

  field_value = rec_field_value (field);
  fail_if(strcmp (field_value, "foo") != 0);

  rec_field_destroy (field);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_value (void)
{
  TCase *tc = tcase_create("rec_field_value");
  tcase_add_test (tc, rec_field_value_empty);
  tcase_add_test (tc, rec_field_value_nonempty);

  return tc;
}

/* End of rec-field-value.c */
