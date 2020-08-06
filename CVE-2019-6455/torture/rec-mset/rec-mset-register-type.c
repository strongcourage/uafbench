/* -*- mode: C -*-
 *
 *       File:         rec-mset-register-type.c
 *       Date:         Thu Oct 28 20:10:18 2010
 *
 *       GNU recutils - Tests for rec_mset_register_type
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
#include <rec-mset/elem-types.h>

/*-
 * Test: rec_mset_register_type_nominal
 * Unit: rec_mset_register_type
 * Description:
 * + Register a type in a mset.
 * +
 * + 1. The registration should succeed.
 * + 2. The number returned by rec_mset_register_type shall
 * +    be bigger than 0.
 */
START_TEST(rec_mset_register_type_nominal)
{
  int type;
  rec_mset_t mset;

  mset = rec_mset_new ();
  fail_if (mset == NULL);
  type = rec_mset_register_type (mset,
                                 TYPE1,
                                 type1_disp,
                                 type1_equal,
                                 type1_dup,
                                 NULL);

  fail_if (type <= 0);
  rec_mset_destroy (mset);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_mset_register_type (void)
{
  TCase *tc = tcase_create ("rec_mset_register_type");
  tcase_add_test (tc, rec_mset_register_type_nominal);

  return tc;
}


/* End of rec-mset-register-type.c */
