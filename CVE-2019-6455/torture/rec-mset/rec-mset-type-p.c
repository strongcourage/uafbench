/* -*- mode: C -*-
 *
 *       File:         rec-mset-type-p.c
 *       Date:         Thu Oct 28 19:55:46 2010
 *
 *       GNU recutils - rec_mset_type_p unit tests
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
 * Test: rec_mset_type_p_any
 * Unit: rec_mset_type_p
 * Description:
 * + Check for the MSET_ANY type with rec_mset_type_p.
 * +
 * + 1. The MSET_ANY type shall exist.
 */
START_TEST(rec_mset_type_p_any)
{
  rec_mset_t mset;

  mset = rec_mset_new ();
  fail_if (mset == NULL);

  fail_if (!rec_mset_type_p (mset, MSET_ANY));
  rec_mset_destroy (mset);
}
END_TEST

/*-
 * Test: rec_mset_type_p_existing
 * Unit: rec_mset_type_p
 * Description:
 * + Register a type in an empty mset and check
 * + for it with rec_mset_type_p.
 * +
 * + 1. The function shall report that the type
 * +    exists.
 */
START_TEST(rec_mset_type_p_existing)
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
  
  fail_if (!rec_mset_type_p (mset, type));
  rec_mset_destroy (mset);
}
END_TEST


/*-
 * Test: rec_mset_type_p_nonexisting
 * Unit: rec_mset_type_p
 * Description:
 * + Register a type in an empty mset and check
 * + for other type with rec_mset_type_p.
 * +
 * + 1. The function shall report that the type
 * +    does not exist.
 */
START_TEST(rec_mset_type_p_nonexisting)
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
  
  fail_if (rec_mset_type_p (mset, type + 1));
  rec_mset_destroy (mset);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_mset_type_p (void)
{
  TCase *tc = tcase_create ("rec_mset_type_p");
  tcase_add_test (tc, rec_mset_type_p_any);
  tcase_add_test (tc, rec_mset_type_p_existing);
  tcase_add_test (tc, rec_mset_type_p_nonexisting);

  return tc;
}

/* End of rec-mset-type-p.c */
