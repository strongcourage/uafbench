/* -*- mode: C -*-
 *
 *       File:         rec-fex-sort.c
 *       Date:         Tue Nov  9 17:51:59 2010
 *
 *       GNU recutils - rec_fex_sort unit tests.
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
 * Test: rec_fex_sort_nominal
 * Unit: rec_fex_sort
 * Description:
 * + Sort a field expression.
 */
START_TEST(rec_fex_sort_nominal)
{
  rec_fex_t fex;
  const char *fname_aaa;
  const char *fname_bbb;
  const char *fname_ccc;

  fname_aaa = "aaa";
  fname_bbb = "bbb";
  fname_ccc = "ccc";

  fex = rec_fex_new ("ccc[2],aaa[0],bbb[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 3);

  rec_fex_sort (fex);

  fail_if (!rec_field_name_equal_p (rec_fex_elem_field_name (rec_fex_get (fex, 0)),
                                    fname_aaa));
  fail_if (!rec_field_name_equal_p (rec_fex_elem_field_name (rec_fex_get (fex, 1)),
                                    fname_bbb));
  fail_if (!rec_field_name_equal_p (rec_fex_elem_field_name (rec_fex_get (fex, 2)),
                                    fname_ccc));

  rec_fex_destroy (fex);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_fex_sort (void)
{
  TCase *tc = tcase_create ("rec_fex_sort");
  tcase_add_test (tc, rec_fex_sort_nominal);

  return tc;
}

/* End of rec-fex-sort.c */
