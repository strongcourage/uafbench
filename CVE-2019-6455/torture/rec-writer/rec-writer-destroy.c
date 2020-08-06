/* -*- mode: C -*-
 *
 *       File:         rec-writer-destroy.c
 *       Date:         Sun Nov 14 10:57:39 2010
 *
 *       GNU recutils - rec_writer_destroy unit tests.
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
#include <stdio.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_writer_destroy_nominal
 * Unit: rec_writer_destroy
 * Description:
 * + Destroy a rec writer.
 */
START_TEST(rec_writer_destroy_nominal)
{
  rec_writer_t writer;

  writer = rec_writer_new (stdout);
  fail_if (writer == NULL);

  rec_writer_destroy (writer);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_writer_destroy (void)
{
  TCase *tc = tcase_create ("rec_writer_destroy");
  tcase_add_test (tc, rec_writer_destroy_nominal);

  return tc;
}

/* End of rec-writer-destroy.c */
