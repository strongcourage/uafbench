/* -*- mode: C -*-
 *
 *       File:         rec-type-descr-p.c
 *       Date:         Fri Oct 29 17:15:50 2010
 *
 *       GNU recutils - rec_type_descr_p unit tests
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
 * Test: rec_type_descr_p_nominal
 * Unit: rec_type_descr_p
 * Description:
 * + Check valid type descriptions.
 */
START_TEST(rec_type_descr_p_nominal)
{
  fail_if (!rec_type_descr_p ("int"));
  fail_if (!rec_type_descr_p ("int  \n\n  "));
  fail_if (!rec_type_descr_p ("bool"));
  fail_if (!rec_type_descr_p ("range 10"));
  fail_if (!rec_type_descr_p ("range 1 10"));
  fail_if (!rec_type_descr_p ("real"));
  fail_if (!rec_type_descr_p ("size 10"));
  fail_if (!rec_type_descr_p ("line"));
  fail_if (!rec_type_descr_p ("regexp /[abc][abc][abc]/"));
  fail_if (!rec_type_descr_p ("regexp |/jo/jo|"));
  fail_if (!rec_type_descr_p ("date"));
  fail_if (!rec_type_descr_p ("enum A B C"));
  fail_if (!rec_type_descr_p ("field"));
  fail_if (!rec_type_descr_p ("email"));
}
END_TEST

/*-
 * Test: rec_type_descr_p_invalid
 * Unit: rec_type_descr_p
 * Description:
 * + Check for invalid type descriptions.
 */
START_TEST(rec_type_descr_p_invalid)
{
  fail_if (rec_type_descr_p (""));
  fail_if (rec_type_descr_p (" "));
  fail_if (rec_type_descr_p ("int additionalstuff"));
  fail_if (rec_type_descr_p ("invalidkeyword"));
  fail_if (rec_type_descr_p ("range a b"));
  fail_if (rec_type_descr_p ("range a 1"));
  fail_if (rec_type_descr_p ("range 1 a"));
  fail_if (rec_type_descr_p ("size"));
  fail_if (rec_type_descr_p ("size xxx"));
  fail_if (rec_type_descr_p ("size 10 extra"));
  fail_if (rec_type_descr_p ("regexp"));
  fail_if (rec_type_descr_p ("regexp foo"));
  fail_if (rec_type_descr_p ("regexp /abc/ extra"));
  fail_if (rec_type_descr_p ("enum"));
  fail_if (rec_type_descr_p ("enum # ! '"));
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_descr_p (void)
{
  TCase *tc = tcase_create ("rec_type_descr_p");
  tcase_add_test (tc, rec_type_descr_p_nominal);
  tcase_add_test (tc, rec_type_descr_p_invalid);

  return tc;
}

/* End of rec-type-descr-p.c */
