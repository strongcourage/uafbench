/* -*- mode: C -*-
 *
 *       File:         rec-mset-new.c
 *       Date:         Thu Oct 28 16:47:46 2010
 *
 *       GNU recutils - rec_mset_new unit tests
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
 * Test: rec_mset_new_and_destroy
 * Unit: rec_mset_new
 * Description:
 * + Create an empty record set and destroy it.
 * +
 * + 1. The value returned by rec_mset_new shall not
 * +    be NULL.
 */
START_TEST(rec_mset_new_and_destroy)
{
  rec_mset_t mset;

  mset = rec_mset_new ();
  fail_if (mset == NULL);
  rec_mset_destroy (mset);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_mset_new (void)
{
  TCase *tc = tcase_create ("rec_mset_new");
  tcase_add_test (tc, rec_mset_new_and_destroy);
  
  return tc;
}

/* End of rec-mset-new.c */
