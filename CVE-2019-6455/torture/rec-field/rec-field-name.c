/* -*- mode: C -*-
 *
 *       File:         rec-field-name.c
 *       Date:         Sun Mar  1 17:04:00 2009
 *
 *       GNU recutils - rec_field_name unit tests
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
 * Test: rec_field_name_empty
 * Unit: rec_field_name
 * Description:
 * + Get the name of a field with an empy name
 * +
 * + 1. The call should not produce an error.
 * + 2. The name of the field should be properly
 * +    returned.
 */
START_TEST(rec_field_name_empty)
{
  rec_field_t field;
  const char *fname;
  
  fname = "";
  field = rec_field_new (fname, "");
  fail_if(field == NULL);

  fname = rec_field_name (field);
  fail_if(strcmp (fname, "") != 0);

  rec_field_destroy (field);
}
END_TEST

/*-
 * Test: rec_field_name_nonempty
 * Unit: rec_field_name
 * Description:
 * + Get the name of a field with a non-empty name
 * +
 * + 1. The call should not produce an error.
 * + 2. The name of the field should be properly
 * +    returned.
 */
START_TEST(rec_field_name_nonempty)
{
  rec_field_t field;
  const char *fname;
  
  fname = "foo";
  field = rec_field_new (fname, "");
  fail_if(field == NULL);

  fname = rec_field_name (field);
  fail_if(strcmp (fname, "foo") != 0);

  rec_field_destroy (field);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_name (void)
{
  TCase *tc = tcase_create("rec_field_name");
  tcase_add_test (tc, rec_field_name_empty);
  tcase_add_test (tc, rec_field_name_nonempty);

  return tc;
}

/* End of rec-field-name.c */
