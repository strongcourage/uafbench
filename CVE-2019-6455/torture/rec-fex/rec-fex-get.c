/* -*- mode: C -*-
 *
 *       File:         rec-fex-get.c
 *       Date:         Wed Nov 10 12:10:43 2010
 *
 *       GNU recutils - rec_fex_get unit tests.
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
 * Test: rec_fex_get_nominal
 * Unit: rec_fex_get
 * Description:
 * + Get elements from a field expression.
 */
START_TEST(rec_fex_get_nominal)
{
  rec_fex_t fex;
  rec_fex_elem_t elem;
  const char *fname_aaa;
  const char *fname_bbb;

  fname_aaa = "aaa";
  fname_bbb = "bbb";

  fex = rec_fex_new ("aaa bbb", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 2);
  
  elem = rec_fex_get (fex, 0);
  fail_if (elem == NULL);
  fail_if (!rec_field_name_equal_p (rec_fex_elem_field_name (elem),
                                    fname_aaa));
  elem = rec_fex_get (fex, 1);
  fail_if (elem == NULL);
  fail_if (!rec_field_name_equal_p (rec_fex_elem_field_name (elem),
                                    fname_bbb));

  rec_fex_destroy (fex);
}
END_TEST

/*-
 * Test: rec_fex_get_invalid
 * Unit: rec_fex_get
 * Description:
 * + Get elements from a field expression
 * + providing invalid positions.
 * +
 * + The calls to 'rec_fex_get' shall return
 * + NULL in those cases.
 */
START_TEST(rec_fex_get_invalid)
{
  rec_fex_t fex;

  fex = rec_fex_new ("aaa bbb ccc", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 3);

  fail_if (rec_fex_get (fex, -10) != NULL);
  fail_if (rec_fex_get (fex, 4) != NULL);
  fail_if (rec_fex_get (fex, 20000) != NULL);

  rec_fex_destroy (fex);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_fex_get (void)
{
  TCase *tc = tcase_create ("rec_fex_get");
  tcase_add_test (tc, rec_fex_get_nominal);
  tcase_add_test (tc, rec_fex_get_invalid);

  return tc;
}

/* End of rec-fex-get.c */
