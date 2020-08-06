/* -*- mode: C -*-
 *
 *       File:         rec-type-destroy.c
 *       Date:         Fri Oct 29 16:57:32 2010
 *
 *       GNU recutils - rec_type_destroy unit tests
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
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_type_destroy_nominal
 * Unit: rec_type_destroy
 * Description:
 * + Destroy a type.
 */
START_TEST(rec_type_destroy_nominal)
{
  rec_type_t type;

  type = rec_type_new ("int");
  fail_if (type == NULL);

  rec_type_destroy (type);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_destroy (void)
{
  TCase *tc = tcase_create ("rec_type_destroy");
  tcase_add_test (tc, rec_type_destroy_nominal);

  return tc;
}

/* End of rec-type-destroy.c */
