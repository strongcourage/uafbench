/* -*- mode: C -*-
 *
 *       File:         rec-field-dup.c
 *       Date:         Thu Mar  5 23:29:56 2009
 *
 *       GNU recutils - rec_field_dup unit tests
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
 * Test: rec_field_dup_empty
 * Unit: rec_field_dup
 * Description:
 * + Dup a field with empty name and empty value.
 * +
 * + 1. The name of the copied field should be the
 * +    empty string.
 * + 2. The value of the copied field should be the
 * +    empty string.
 */
START_TEST(rec_field_dup_empty)
{
  rec_field_t field;
  rec_field_t field_copy;
  const char *field_name;
  const char *field_value;

  /* Create a field */
  field_name = "";
  field = rec_field_new (field_name, "");
  fail_if(field == NULL);

  /* Create a copy of the field */
  field_copy = rec_field_dup (field);
  fail_if(field_copy == NULL);

  /* Get the attributes of the copy */
  field_name = rec_field_name (field_copy);
  field_value = rec_field_value (field_copy);

  /* Validate the attributes */
  fail_if(strcmp (field_name, "") != 0);
  fail_if(strcmp (field_value, "") != 0);

  rec_field_destroy (field);
  rec_field_destroy (field_copy);
}
END_TEST

/*-
 * Test: rec_field_dup_non_empty
 * Unit: rec_field_dup
 * Description:
 * + Dup a field with non-empty name and non-empty value.
 * +
 * + 1. The name of the copied field should be the
 * +    appropriate.
 * + 2. The value of the copied field should be the
 * +    appropriate.
 */
START_TEST(rec_field_dup_nonempty)
{
  rec_field_t field;
  rec_field_t field_copy;
  const char *field_name;
  const char *field_value;

  /* Create a field */
  field_name = "name";
  field = rec_field_new (field_name, "value");
  fail_if(field == NULL);

  /* Create a copy of the field */
  field_copy = rec_field_dup (field);
  fail_if(field_copy == NULL);

  /* Get the attributes of the copy */
  field_name = rec_field_name (field_copy);
  field_value = rec_field_value (field_copy);

  /* Validate the attributes */
  fail_if(strcmp (field_name, "name") != 0);
  fail_if(strcmp (field_value, "value") != 0);

  rec_field_destroy (field);
  rec_field_destroy (field_copy);
}
END_TEST

/* 
 * Test case creation function
 */
TCase *
test_rec_field_dup (void)
{
  TCase *tc = tcase_create("rec_field_dup");
  tcase_add_test (tc, rec_field_dup_empty);
  tcase_add_test (tc, rec_field_dup_nonempty);

  return tc;
}

/* End of rec-field-dup.c */
