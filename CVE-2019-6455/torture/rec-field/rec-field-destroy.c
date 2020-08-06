/* -*- mode: C -*-
 *
 *       File:         rec-field-destroy.c
 *       Date:         Fri Nov 12 13:19:45 2010
 *
 *       GNU recutils - rec_field_destroy unit tests.
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
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_field_destroy_nominal
 * Unit: rec_field_destroy
 * Description:
 * + Destroy a field.
 */
START_TEST(rec_field_destroy_nominal)
{
  rec_field_t field;

  field = rec_field_new ("foo", "value");
  fail_if (field == NULL);

  rec_field_destroy (field);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_field_destroy (void)
{
  TCase *tc = tcase_create ("rec_field_new");
  tcase_add_test (tc, rec_field_destroy_nominal);
  
  return tc;
}

/* End of rec-field-destroy.c */
