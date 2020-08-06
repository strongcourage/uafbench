/* -*- mode: C -*-
 *
 *       File:         rec-type-kind.c
 *       Date:         Fri Oct 29 18:08:46 2010
 *
 *       GNU recutils - rec_type_kind unit tests
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
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_type_kind_nominal
 * Unit: rec_type_kind
 * Description:
 * + Get the kind of types.
 */
START_TEST(rec_type_kind_nominal)
{
  rec_type_t type;

  type = rec_type_new ("int");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_INT);
  rec_type_destroy (type);

  type = rec_type_new ("bool");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_BOOL);
  rec_type_destroy (type);

  type = rec_type_new ("range 1 10");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_RANGE);
  rec_type_destroy (type);

  type = rec_type_new ("real");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_REAL);
  rec_type_destroy (type);

  type = rec_type_new ("size 10");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_SIZE);
  rec_type_destroy (type);

  type = rec_type_new ("line");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_LINE);
  rec_type_destroy (type);

  type = rec_type_new ("regexp /[abc]/");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_REGEXP);
  rec_type_destroy (type);

  type = rec_type_new ("date");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_DATE);
  rec_type_destroy (type);

  type = rec_type_new ("enum A B C");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_ENUM);
  rec_type_destroy (type);

  type = rec_type_new ("field");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_FIELD);
  rec_type_destroy (type);

  type = rec_type_new ("email");
  fail_if (type == NULL);
  fail_if (rec_type_kind (type) != REC_TYPE_EMAIL);
  rec_type_destroy (type);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_kind (void)
{
  TCase *tc = tcase_create ("rec_type_kind");
  tcase_add_test (tc, rec_type_kind_nominal);

  return tc;
}

/* End of rec-type-kind.c */
