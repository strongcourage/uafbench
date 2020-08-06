/* -*- mode: C -*-
 *
 *       File:         rec-fex-check.c
 *       Date:         Tue Nov  9 17:26:40 2010
 *
 *       GNU recutils - rec_fex_check unit tests.
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
 * Test: rec_fex_check_simple_nominal
 * Unit: rec_fex_check
 * Description:
 * + Check for simple field expressions.
 */
START_TEST(rec_fex_check_simple_nominal)
{
  fail_if (!rec_fex_check ("foo", REC_FEX_SIMPLE));
  fail_if (!rec_fex_check ("foo bar baz", REC_FEX_SIMPLE));
  fail_if (!rec_fex_check ("foo\nbar\nbaz", REC_FEX_SIMPLE));
  fail_if (!rec_fex_check ("foo\tbar\tbaz", REC_FEX_SIMPLE));
}
END_TEST

/*-
 * Test: rec_fex_check_simple_invalid
 * Unit: rec_fex_check
 * Description:
 * + Check for simple field expressions.
 */
START_TEST(rec_fex_check_simple_invalid)
{
  fail_if (rec_fex_check ("fo!o", REC_FEX_SIMPLE));
  fail_if (rec_fex_check ("foo,bar", REC_FEX_SIMPLE));
}
END_TEST

/*-
 * Test: rec_fex_check_csv_nominal
 * Unit: rec_fex_check
 * Description:
 * + Check for comma-separated simple field expressions.
 */
START_TEST(rec_fex_check_csv_nominal)
{
  fail_if (!rec_fex_check ("foo", REC_FEX_CSV));
  fail_if (!rec_fex_check ("foo,bar,baz", REC_FEX_CSV));
  fail_if (!rec_fex_check ("foobarbaz,bar,baz", REC_FEX_CSV));
}
END_TEST

/*-
 * Test: rec_fex_check_csv_invalid
 * Unit: rec_fex_check
 * Description:
 * + Check for csv simple field expressions.
 */
START_TEST(rec_fex_check_csv_invalid)
{
  fail_if (rec_fex_check ("fo!o", REC_FEX_CSV));
  fail_if (rec_fex_check ("foo bar", REC_FEX_CSV));
}
END_TEST

/*-
 * Test: rec_fex_check_sub_nominal
 * Unit: rec_fex_check
 * Description:
 * + Check for comma-separated field expressions.
 */
START_TEST(rec_fex_check_sub_nominal)
{
  fail_if (!rec_fex_check ("foo", REC_FEX_SUBSCRIPTS));
  fail_if (!rec_fex_check ("foo[10]", REC_FEX_SUBSCRIPTS));
  fail_if (!rec_fex_check ("foo,bar,baz", REC_FEX_SUBSCRIPTS));
  fail_if (!rec_fex_check ("foo[0],bar[1],baz[2]", REC_FEX_SUBSCRIPTS));
  fail_if (!rec_fex_check ("foo[0-10],bar[1-100],baz[2-20]", REC_FEX_SUBSCRIPTS));
  fail_if (!rec_fex_check ("foobarbaz,bar,baz", REC_FEX_SUBSCRIPTS));
  fail_if (!rec_fex_check ("foobarbaz,bar[100-0],baz", REC_FEX_SUBSCRIPTS));
}
END_TEST

/*-
 * Test: rec_fex_check_sub_invalid
 * Unit: rec_fex_check
 * Description:
 * + Check for comma-separated field expressions.
 */
START_TEST(rec_fex_check_sub_invalid)
{
  fail_if (rec_fex_check ("fo!o", REC_FEX_SUBSCRIPTS));
  fail_if (rec_fex_check ("foo[]", REC_FEX_SUBSCRIPTS));
  fail_if (rec_fex_check ("foo bar", REC_FEX_SUBSCRIPTS));
  fail_if (rec_fex_check ("foo,bar[[10]", REC_FEX_SUBSCRIPTS));
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_fex_check (void)
{
  TCase *tc = tcase_create ("rec_fex_check");
  tcase_add_test (tc, rec_fex_check_simple_nominal);
  tcase_add_test (tc, rec_fex_check_simple_invalid);
  tcase_add_test (tc, rec_fex_check_csv_nominal);
  tcase_add_test (tc, rec_fex_check_csv_invalid);
  tcase_add_test (tc, rec_fex_check_sub_nominal);
  tcase_add_test (tc, rec_fex_check_sub_invalid);

  return tc;
}

/* End of rec-fex-check.c  */
