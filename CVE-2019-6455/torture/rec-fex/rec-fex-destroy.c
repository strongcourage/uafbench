/* -*- mode: C -*-
 *
 *       File:         rec-fex-destroy.c
 *       Date:         Tue Nov  9 15:54:08 2010
 *
 *       GNU recutils - rec_fex_destroy unit tests.
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
 * Test: rec_fex_destroy_nominal
 * Unit: rec_fex_destroy
 * Description:
 * + Destroy a properly initialized field expression.
 */
START_TEST(rec_fex_destroy_nominal)
{
  rec_fex_t fex;

  fex = rec_fex_new ("foo", REC_FEX_SIMPLE);
  fail_if (fex == NULL);

  rec_fex_destroy (fex);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_fex_destroy (void)
{
  TCase *tc = tcase_create ("rec_fex_new");
  tcase_add_test (tc, rec_fex_destroy_nominal);

  return tc;
}

/* End of rec-fex-destroy.c */
