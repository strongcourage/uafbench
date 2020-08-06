/* -*- mode: C -*-
 *
 *       File:         rec-type-reg-get.c
 *       Date:         Fri Oct 29 21:30:27 2010
 *
 *       GNU recutils - rec_type_reg_get unit tests
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
 * Test: rec_type_reg_get_nominal
 * Unit: rec_type_reg_get
 * Description:
 * + Insert a type in a type registry and get it.
 * + The call shall success.
 */
START_TEST(rec_type_reg_get_nominal)
{
  rec_type_t type;
  rec_type_t type2;
  rec_type_reg_t reg;

  reg = rec_type_reg_new ();
  fail_if (reg == NULL);

  /* Register a type.  */
  type = rec_type_new ("int");
  rec_type_set_name (type, "foo");
  fail_if (type == NULL);
  rec_type_reg_add (reg, type);

  /* Get the type and compare.  */
  type2 = rec_type_reg_get (reg, "foo");
  fail_if (type2 == NULL);
  fail_if (type2 != type);

  rec_type_reg_destroy (reg);
}
END_TEST

/*-
 * Test: rec_type_reg_get_nonexisting
 * Unit: rec_type_reg_get
 * Description:
 * + Try to get a nonexisting named type
 * + from a type registry.
 */
START_TEST(rec_type_reg_get_nonexisting)
{
  rec_type_reg_t reg;

  reg = rec_type_reg_new ();
  fail_if (reg == NULL);

  fail_if (rec_type_reg_get (reg, "foo") != NULL);
  
  rec_type_reg_destroy (reg);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_reg_get (void)
{
  TCase *tc = tcase_create ("rec_type_reg_get");
  tcase_add_test (tc, rec_type_reg_get_nominal);
  tcase_add_test (tc, rec_type_reg_get_nonexisting);

  return tc;
}

/* End of rec-type-reg-get.c */
