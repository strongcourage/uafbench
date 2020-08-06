/* -*- mode: C -*-
 *
 *       File:         rec-fex-elem-max.c
 *       Date:         Fri Nov 12 12:38:03 2010
 *
 *       GNU recutils - rec_fex_elem_max unit tests.
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
 * Test: rec_fex_elem_max_nosub
 * Unit: rec_fex_elem_max
 * Description:
 * + Get the max index of an entry in
 * + a field expression created without
 * + subscripts.
 * +
 * + The max subscript shall be -1 in this case.
 */
START_TEST(rec_fex_elem_max_nosub)
{
  rec_fex_t fex;
  rec_fex_elem_t elem;

  fex = rec_fex_new ("foo bar baz", REC_FEX_SIMPLE);
  fail_if (fex == NULL);

  elem = rec_fex_get (fex, 1);
  fail_if (elem == NULL);
  fail_if (rec_fex_elem_max (elem) != -1);

  rec_fex_destroy (fex);
}
END_TEST

/*-
 * Test: rec_fex_elem_max_min
 * Unit: rec_fex_elem_max
 * Description:
 * + Get the max index of an entry in
 * + a field expression created with
 * + a min subscript.
 */
START_TEST(rec_fex_elem_max_min)
{
  rec_fex_t fex;
  rec_fex_elem_t elem;
  
  fex = rec_fex_new ("foo,bar[6],baz", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);

  elem = rec_fex_get (fex, 1);
  fail_if (elem == NULL);
  fail_if (rec_fex_elem_max (elem) != -1);

  rec_fex_destroy (fex);
}
END_TEST

/*-
 * Test: rec_fex_elem_max_minmax
 * Unit: rec_fex_elem_max
 * Description:
 * + Get the max index of an entry in
 * + a field expression created with
 * + full subscripts (both min and max).
 */
START_TEST(rec_fex_elem_max_minmax)
{
  rec_fex_t fex;
  rec_fex_elem_t elem;

  fex = rec_fex_new ("foo,bar[2-10],baz", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);

  elem = rec_fex_get (fex, 1);
  fail_if (elem == NULL);
  fail_if (rec_fex_elem_max (elem) != 10);

  rec_fex_destroy (fex);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_fex_elem_max (void)
{
  TCase *tc = tcase_create ("rec_fex_elem_max");
  tcase_add_test (tc, rec_fex_elem_max_nosub);
  tcase_add_test (tc, rec_fex_elem_max_min);
  tcase_add_test (tc, rec_fex_elem_max_minmax);

  return tc;
}

/* End of rec-fex-elem-max.c */
