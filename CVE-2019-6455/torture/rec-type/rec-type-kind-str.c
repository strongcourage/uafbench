/* -*- mode: C -*-
 *
 *       File:         rec-type-kind-str.c
 *       Date:         Fri Oct 29 18:17:18 2010
 *
 *       GNU recutils - rec_type_kind_str unit tests
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
 * Test: rec_type_kind_str_nominal
 * Unit: rec_type_kind_str
 * Description:
 * + Get the string representation of all
 * + the supported types and check they are
 * + ok.
 */
START_TEST(rec_type_kind_str_nominal)
{
  rec_type_t type;

  type = rec_type_new ("int");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "int") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("bool");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "bool") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("range 1 10");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "range") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("real");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "real") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("size 10");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "size") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("line");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "line") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("regexp /[abc]/");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "regexp") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("date");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "date") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("enum A B C");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "enum") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("email");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "email") != 0);
  rec_type_destroy (type);

  type = rec_type_new ("field");
  fail_if (type == NULL);
  fail_if (strcmp (rec_type_kind_str (type), "field") != 0);
  rec_type_destroy (type);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_kind_str (void)
{
  TCase *tc = tcase_create ("rec_type_kind_str");
  tcase_add_test (tc, rec_type_kind_str_nominal);

  return tc;
}

/* End of rec-type-kind-str.c */
