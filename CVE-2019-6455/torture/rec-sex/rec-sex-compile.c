/* -*- mode: C -*-
 *
 *       File:         rec-sex-compile.c
 *       Date:         Mon Nov 15 15:12:24 2010
 *
 *       GNU recutils - rec_sex_compile unit tests.
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
 * Test: rec_sex_compile_nominal
 * Unit: rec_sex_compile
 * Description:
 * + Compile valid selection expressions.
 */
START_TEST(rec_sex_compile_nominal)
{
  rec_sex_t sex;

  sex = rec_sex_new (false);
  fail_if (sex == NULL);
  fail_if (!rec_sex_compile (sex,
                             "true"));
  rec_sex_destroy (sex);

  sex = rec_sex_new (false);
  fail_if (sex == NULL);
  fail_if (!rec_sex_compile (sex,
                             "false"));
  rec_sex_destroy (sex);
}
END_TEST

/*-
 * Test: rec_sex_compile_invalid
 * Unit: rec_sex_compile
 * Description:
 * + Try to compile invalid selection expressions.
 */
START_TEST(rec_sex_compile_invalid)
{
  rec_sex_t sex;

  sex = rec_sex_new (false);
  fail_if (sex == NULL);
  fail_if (rec_sex_compile (sex, ""));
  rec_sex_destroy (sex);

  sex = rec_sex_new (false);
  fail_if (sex == NULL);
  fail_if (rec_sex_compile (sex, "+"));
  rec_sex_destroy (sex);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_sex_compile (void)
{
  TCase *tc = tcase_create ("rec_sex_compile");
  tcase_add_test (tc, rec_sex_compile_nominal);
  tcase_add_test (tc, rec_sex_compile_invalid);

  return tc;
}

/* End of rec-sex-compile.c */
