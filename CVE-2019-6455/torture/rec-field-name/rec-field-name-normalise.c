/* -*- mode: C -*-
 *
 *       File:         rec-field-name-normalise.c
 *       Date:         Fri Oct 29 15:38:54 2010
 *
 *       GNU recutils - rec_field_name_normalise unit tests
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
#include <stdlib.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_field_name_normalise_nominal
 * Unit: rec_field_name_normalise
 * Description:
 * + Check normalisation of names.
 */
START_TEST(rec_field_name_normalise_nominal)
{
  char *normalised;

  normalised = rec_field_name_normalise ("abc");
  fail_if (normalised == NULL);
  fail_if (strcmp (normalised, "abc") != 0);
  free (normalised);
  
  normalised = rec_field_name_normalise ("a#c d");
  fail_if (normalised == NULL);
  fail_if (strcmp (normalised, "a_c_d") != 0);
  free (normalised);

  normalised = rec_field_name_normalise ("a-b%c_d");
  fail_if (normalised != NULL);
  free (normalised);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_name_normalise (void)
{
  TCase *tc = tcase_create ("rec_field_name_normalise");
  tcase_add_test (tc, rec_field_name_normalise_nominal);

  return tc;
}

/* End of rec-field-name-normalise.c */
