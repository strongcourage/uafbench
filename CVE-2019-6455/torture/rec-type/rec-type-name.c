/* -*- mode: C -*-
 *
 *       File:         rec-type-name.c
 *       Date:         Thu Jul 14 21:36:05 2011
 *
 *       GNU recutils - rec_type_name unit tests
 *
 */

/* Copyright (C) 2011-2015 Jose E. Marchesi */

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
#include <stdlib.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_type_name_anonymous
 * Unit: rec_type_name
 * Description:
 * + Get the name of an anonymous type.
 * + The returned value shall be NULL.
 */
START_TEST(rec_type_name_anonymous)
{
  rec_type_t type;

  type = rec_type_new ("int");
  fail_if (type == NULL);
  fail_if (rec_type_name (type) != NULL);

  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_name_nominal
 * Unit: rec_type_name
 * Description:
 * + Get the name of a type.
 * + The returned name shall be correct.
 */
START_TEST(rec_type_name_nominal)
{
  rec_type_t type;

  type = rec_type_new ("range 0 120");
  fail_if (type == NULL);
  rec_type_set_name (type, "age_t");
  fail_if (strcmp ("age_t",
                   rec_type_name (type)) != 0);

  rec_type_destroy (type);
}
END_TEST

/*
 * Test case creation function.
 */
TCase *
test_rec_type_name (void)
{
  TCase *tc = tcase_create ("rec_type_name");
  tcase_add_test (tc, rec_type_name_anonymous);
  tcase_add_test (tc, rec_type_name_nominal);

  return tc;
}

/* End of rec-type-name.c */
