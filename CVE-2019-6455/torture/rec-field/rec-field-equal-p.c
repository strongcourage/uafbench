/* -*- mode: C -*-
 *
 *       File:         rec-field-equal-p.c
 *       Date:         Fri Nov 12 13:34:34 2010
 *
 *       GNU recutils - rec_field_equal_p unit tests.
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
 * Test: rec_field_equal_p_equal
 * Unit: rec_field_equal_p
 * Description:
 * + Test if two given fields are equal. i.e.
 * + they have eql field names and the same value.
 */
START_TEST(rec_field_equal_p_equal)
{
  rec_field_t field1;
  rec_field_t field2;

  field1 = rec_field_new ("foo", "value");
  fail_if (field1 == NULL);

  field2 = rec_field_new ("foo", "value");
  fail_if (field2 == NULL);

  fail_if (!rec_field_equal_p (field1, field2));

  rec_field_destroy (field1);
  rec_field_destroy (field2);
}
END_TEST

/*-
 * Test: rec_field_equal_p_nonequal
 * Unit: rec_field_equal_p
 * Description:
 * + Check whether two different fields are
 * + equal.
 * +
 * + The comparison function shall return false.
 */
START_TEST(rec_field_equal_p_nonequal)
{
  rec_field_t field1;
  rec_field_t field2;

  field1 = rec_field_new ("foo", "value");
  fail_if (field1 == NULL);
  field2 = rec_field_new ("other", "value");
  fail_if (field2 == NULL);

  fail_if (rec_field_equal_p (field1, field2));

  rec_field_destroy (field1);
  rec_field_destroy (field2);
}
END_TEST

/*
 * Test creation function.
 */
TCase *
test_rec_field_equal_p (void)
{
  TCase *tc = tcase_create ("rec_field_equal_p");
  tcase_add_test (tc, rec_field_equal_p_equal);
  tcase_add_test (tc, rec_field_equal_p_nonequal);

  return tc;
}

/* End of rec-field-equal-p.c */
