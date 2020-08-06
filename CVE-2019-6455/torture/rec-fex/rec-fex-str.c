/* -*- mode: C -*-
 *
 *       File:         rec-fex-str.c
 *       Date:         Thu Nov 25 16:13:12 2010
 *
 *       GNU recutils - rec_fex_str unit tests.
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
#include <stdlib.h>

#include <rec.h>

/*-
 * Test: rec_fex_str_nominal
 * Unit: rec_fex_str
 * Description:
 * + Write fexes in strings.
 */
START_TEST(rec_fex_str_nominal)
{
  rec_fex_t fex;
  char *str;

  fex = rec_fex_new ("foo", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  str = rec_fex_str (fex, REC_FEX_SIMPLE);
  fail_if (str == NULL);
  fail_if (strcmp (str, "foo") != 0);
  free (str);
  str = rec_fex_str (fex, REC_FEX_CSV);
  fail_if (str == NULL);
  fail_if (strcmp (str, "foo") != 0);
  free (str);
  str = rec_fex_str (fex, REC_FEX_SUBSCRIPTS);
  fail_if (str == NULL);
  fail_if (strcmp (str, "foo") != 0);
  free (str);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("foo bar baz", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  str = rec_fex_str (fex, REC_FEX_SIMPLE);
  fail_if (str == NULL);
  fail_if (strcmp (str, "foo bar baz") != 0);
  free (str);
  str = rec_fex_str (fex, REC_FEX_CSV);
  fail_if (str == NULL);
  fail_if (strcmp (str, "foo,bar,baz") != 0);
  free (str);
  str = rec_fex_str (fex, REC_FEX_SUBSCRIPTS);
  fail_if (str == NULL);
  fail_if (strcmp (str, "foo,bar,baz") != 0);
  free (str);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("foo[0],bar[1-2],baz", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  str = rec_fex_str (fex, REC_FEX_SIMPLE);
  fail_if (str == NULL);
  fail_if (strcmp (str, "foo bar baz") != 0);
  free (str);
  str = rec_fex_str (fex, REC_FEX_CSV);
  fail_if (str == NULL);
  fail_if (strcmp (str, "foo,bar,baz") != 0);
  free (str);
  str = rec_fex_str (fex, REC_FEX_SUBSCRIPTS);
  fail_if (str == NULL);
  fail_if (strcmp (str, "foo[0],bar[1-2],baz") != 0);
  free (str);
  rec_fex_destroy (fex);
}
END_TEST


/*
 * Test case creation function
 */
TCase *
test_rec_fex_str (void)
{
  TCase *tc = tcase_create ("rec_fex_str");
  tcase_add_test (tc, rec_fex_str_nominal);

  return tc;
}

/* End of rec-fex-str.c */
