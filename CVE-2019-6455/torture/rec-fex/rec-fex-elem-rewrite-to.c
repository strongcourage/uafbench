/* -*- mode: C -*- Time-stamp: "2015-04-27 19:59:28 jemarch"
 *
 *       File:         rec-fex-elem-rewrite-to.c
 *       Date:         Sun Feb 26 13:14:15 2012
 *
 *       GNU recutils - rec_fex_elem_rewrite_to unit tests.
 *
 */

/* Copyright (C) 2012-2015 Jose E. Marchesi */

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
 * Test: rec_fex_elem_rewrite_to_default
 * Unit: rec_fex_elem_rewrite_to
 * Description:
 * + The rewrite_to property of a fex
 * + elem without a rewrite rule must be NULL.
 */
START_TEST(rec_fex_elem_rewrite_to_default)
{
  rec_fex_t fex;
  rec_fex_elem_t elem;
  
  fex = rec_fex_new ("foo,bar,baz", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  
  elem = rec_fex_get (fex, 0);
  fail_if (elem == NULL);
  fail_if (rec_fex_elem_rewrite_to (elem) != NULL);

  rec_fex_destroy (fex);
}
END_TEST

/*-
 * Test: rec_fex_elem_rewrite_to_nominal
 * Unit: rec_fex_elem_rewrite_to
 * Description:
 * + The rewrite_to property of a fex
 * + elem must be properly parsed and returned.
 */
START_TEST(rec_fex_elem_rewrite_to_nominal)
{
  rec_fex_t fex;
  rec_fex_elem_t elem;
  
  fex = rec_fex_new ("foo,bar:xxx,baz", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  
  elem = rec_fex_get (fex, 1);
  fail_if (elem == NULL);
  fail_if (rec_fex_elem_rewrite_to (elem) == NULL);
  fail_if (strcmp (rec_fex_elem_rewrite_to (elem), "xxx") != 0);

  rec_fex_destroy (fex);
}
END_TEST

/*
 * Test case creation function
 */

TCase *
test_rec_fex_elem_rewrite_to (void)
{
  TCase *tc = tcase_create ("rec_fex_elem_rewrite_to");
  tcase_add_test (tc, rec_fex_elem_rewrite_to_default);
  tcase_add_test (tc, rec_fex_elem_rewrite_to_nominal);

  return tc;
}

/* End of rec-fex-elem-rewrite-to.c */
