/* -*- mode: C -*-
 *
 *       File:         rec-mset-dup.c
 *       Date:         Thu Oct 28 19:03:01 2010
 *
 *       GNU recutils - rec_mset_dup unit tests
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
#include <rec-mset/elem-types.h>

/*-
 * Test: rec_mset_dup_empty
 * Unit: rec_mset_dup
 * Description:
 * + Dup an empty mset.
 * +
 * + 1. The copy shall be successfully returned.
 * + 2. The copy shall be empty.
 */
START_TEST(rec_mset_dup_empty)
{
  rec_mset_t mset1;
  rec_mset_t mset2;

  /* Create an empty mset.  */
  mset1 = rec_mset_new ();
  fail_if (mset1 == NULL);

  /* Get a copy.  */
  mset2 = rec_mset_dup (mset1);
  fail_if (mset2 == NULL);
  fail_if (rec_mset_count (mset2, MSET_ANY) != 0);

  rec_mset_destroy (mset1);
  rec_mset_destroy (mset2);
}
END_TEST

/*-
 * Test: rec_mset_dup_nonempty
 * Unit: rec_mset_dup
 * Description:
 * + Dup an mset containing two elements of a nonempty type.
 * +
 * + 1. The copy shall be successfully returned.
 * + 2. The copy shall contain the same elements.
 */
START_TEST(rec_mset_dup_nonempty)
{
  int type;
  struct type1_t *elem1;
  struct type1_t *elem2;
  rec_mset_elem_t e1;
  rec_mset_elem_t e2;
  rec_mset_t mset1;
  rec_mset_t mset2;

  /* Create a mset, register a type and insert two elements of that
     type.  */
  mset1 = rec_mset_new ();
  fail_if (mset1 == NULL);
  type = rec_mset_register_type (mset1,
                                 TYPE1,
                                 type1_disp,
                                 type1_equal,
                                 type1_dup,
                                 NULL);
  elem1 = malloc (sizeof (struct type1_t));
  fail_if (elem1 == NULL);
  elem1->i = 1;
  e1 = rec_mset_append (mset1, type, (void *) elem1, MSET_ANY);
  fail_if (e1 == NULL);

  elem2 = malloc (sizeof (struct type1_t));
  fail_if (elem2 == NULL);
  elem2->i = 2;
  e2 = rec_mset_append (mset1, type, (void *) elem2, MSET_ANY);
  fail_if (e2 == NULL);

  /* Make a copy of the mset.  */
  mset2 = rec_mset_dup (mset1);
  fail_if (mset2 == NULL);
  fail_if (rec_mset_count (mset2, type) != 2);
  
  rec_mset_destroy (mset1);
  rec_mset_destroy (mset2);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_mset_dup (void)
{
  TCase *tc = tcase_create ("rec_mset_dup");
  tcase_add_test (tc, rec_mset_dup_empty);
  tcase_add_test (tc, rec_mset_dup_nonempty);
  
  return tc;
}

/* End of rec-mset-dup.c */
