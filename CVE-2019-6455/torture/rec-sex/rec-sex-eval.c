/* -*- mode: C -*-
 *
 *       File:         rec-sex-eval.c
 *       Date:         Mon Nov 15 15:19:22 2010
 *
 *       GNU recutils - rec_sex_eval unit tests.
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
 * Test: rec_sex_eval_match
 * Unit: rec_sex_eval
 * Description:
 * + Eval SEX expressions against matching records.
 */
START_TEST(rec_sex_eval_match)
{
  rec_sex_t sex;

  sex = rec_sex_new (false);
  fail_if (sex == NULL);

  rec_sex_destroy (sex);
}
END_TEST

/*-
 * Test: rec_sex_eval_nomatch
 * Unit: rec_sex_eval
 * Description:
 * + Eval SEX expressions against non matching
 * + records.
 */
START_TEST(rec_sex_eval_nomatch)
{
  rec_sex_t sex;

  sex = rec_sex_new (false);
  fail_if (sex == NULL);

  rec_sex_destroy (sex);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_sex_eval (void)
{
  TCase *tc = tcase_create ("rec_sex_eval");
  tcase_add_test (tc, rec_sex_eval_match);
  tcase_add_test (tc, rec_sex_eval_nomatch);

  return tc;
}

/* End of rec-sex-eval.c */
