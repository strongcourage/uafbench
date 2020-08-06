/* -*- mode: C -*-
 *
 *       File:         rec-type-new.c
 *       Date:         Fri Oct 29 16:16:05 2010
 *
 *       GNU recutils - rec_type_new unit tests
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
 * Test: rec_type_new_nominal
 * Unit: rec_type_new
 * Description:
 * + Create types from correct type descriptions.
 */
START_TEST(rec_type_new_nominal)
{
  rec_type_t type;

  type = rec_type_new ("int");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("int   \n\n  ");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("bool");
  fail_if (type == NULL);
  rec_type_destroy (type);
  
  type = rec_type_new ("range 10");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("range 1 10");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("real");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("size 10");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("line");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("regexp /[abc][abc][abc]/");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("regexp |/jo/jo|");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("date");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("enum A B C");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("field");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("email");
  fail_if (type == NULL);
  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_new_invalid
 * Unit: rec_type_new
 * Description:
 * + Try to create types from invalid type descriptions.
 */
START_TEST(rec_type_new_invalid)
{
  rec_type_t type;

  type = rec_type_new ("");
  fail_if (type != NULL);

  type = rec_type_new (" ");
  fail_if (type != NULL);

  type = rec_type_new ("int additionalstuff");
  fail_if (type != NULL);
  
  type = rec_type_new ("invalidkeyword");
  fail_if (type != NULL);

  type = rec_type_new ("range");
  fail_if (type != NULL);

  type = rec_type_new ("range a");
  fail_if (type != NULL);

  type = rec_type_new ("range a b");
  fail_if (type != NULL);

  type = rec_type_new ("range a 1");
  fail_if (type != NULL);

  type = rec_type_new ("range 1 a");
  fail_if (type != NULL);

  type = rec_type_new ("size");
  fail_if (type != NULL);

  type = rec_type_new ("size xxx");
  fail_if (type != NULL);

  type = rec_type_new ("size 10 extra");
  fail_if (type != NULL);

  type = rec_type_new ("regexp");
  fail_if (type != NULL);

  type = rec_type_new ("regexp foo");
  fail_if (type != NULL);

  type = rec_type_new ("regexp /abc/ extra");
  fail_if (type != NULL);

  type = rec_type_new ("enum");
  fail_if (type != NULL);

  type = rec_type_new ("enum  ");
  fail_if (type != NULL);

  type = rec_type_new ("enum # ! '");
  fail_if (type != NULL);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_new (void)
{
  TCase *tc = tcase_create ("rec_type_new");
  tcase_add_test (tc, rec_type_new_nominal);
  tcase_add_test (tc, rec_type_new_invalid);

  return tc;
}

/* End of rec-type-new.c */
