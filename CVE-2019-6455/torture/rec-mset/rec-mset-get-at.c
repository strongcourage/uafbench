/* -*- mode: C -*-
 *
 *       File:         rec-mset-get-at.c
 *       Date:         Thu Oct 28 20:35:33 2010
 *
 *       GNU recutils - Unit tests for rec_mset_get_at
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
 * Test: rec_mset_get_at_empty
 * Unit: rec_mset_get_at
 * Description:
 * + Try to get an element from an empty mset.
 * +
 * + 1. The function shall return NULL.
 */
START_TEST(rec_mset_get_at_empty)
{
  rec_mset_t mset;

  mset = rec_mset_new ();
  fail_if (mset == NULL);
  fail_if (rec_mset_get_at (mset, MSET_ANY, 0) != NULL);

  rec_mset_destroy (mset);
}
END_TEST

/*-
 * Test: rec_mset_get_at_existing
 * Unit: rec_mset_get
 * Description:
 * + Get an existing element from a mset.
 * +
 * + 1. The function shall return a pointer to
 * +    the element.
 */
START_TEST(rec_mset_get_at_existing)
{
  int type;
  struct type1_t *elem1, *elem1aux;
  rec_mset_elem_t e1;
  rec_mset_t mset;

  /* Create a mset, register a type and insert two elements of that
     type.  */
  mset = rec_mset_new ();
  fail_if (mset == NULL);
  type = rec_mset_register_type (mset,
                                 TYPE1,
                                 type1_disp,
                                 type1_equal,
                                 type1_dup,
                                 NULL);
  elem1 = malloc (sizeof (struct type1_t));
  fail_if (elem1 == NULL);
  elem1->i = 1;
  e1 = rec_mset_append (mset, type, (void *) elem1, MSET_ANY);
  fail_if (e1 == NULL);

  /* Get the element and compare.  */
  elem1aux = rec_mset_get_at (mset, type, 0);
  fail_if (elem1aux == NULL);
  fail_if (elem1aux != elem1);

  rec_mset_destroy (mset);
}
END_TEST

/*-
 * Test: rec_mset_get_at_any
 * Unit: rec_mset_get
 * Description:
 * + Get an existing element from a mset using
 * + the ANY index.
 * +
 * + 1. The function shall return a pointer to
 * +    the element.
 */
START_TEST(rec_mset_get_at_any)
{
  int type;
  struct type1_t *elem1, *elem1aux;
  rec_mset_elem_t e1;
  rec_mset_t mset;

  /* Create a mset, register a type and insert two elements of that
     type.  */
  mset = rec_mset_new ();
  fail_if (mset == NULL);
  type = rec_mset_register_type (mset,
                                 TYPE1,
                                 type1_disp,
                                 type1_equal,
                                 type1_dup,
                                 NULL);
  elem1 = malloc (sizeof (struct type1_t));
  fail_if (elem1 == NULL);
  elem1->i = 1;
  e1 = rec_mset_append (mset, type, (void *) elem1, MSET_ANY);
  fail_if (e1 == NULL);

  /* Get the element and compare.  */
  elem1aux = rec_mset_get_at (mset, MSET_ANY, 0);
  fail_if (elem1aux == NULL);
  fail_if (elem1aux != elem1);

  rec_mset_destroy (mset);
}
END_TEST

/*-
 * Test: rec_mset_get_at_invalid
 * Unit: rec_mset_get_at
 * Description:
 * + Get the first element of a mset by using invalid
 * + indexes.
 * +
 * + 1. The function shall return NULL.
 */
START_TEST(rec_mset_get_at_invalid)
{
  int type;
  struct type1_t *elem1;
  rec_mset_elem_t e1;
  rec_mset_elem_t e1aux;
  rec_mset_t mset;

  /* Create a mset, register a type and insert two elements of that
     type.  */
  mset = rec_mset_new ();
  fail_if (mset == NULL);
  type = rec_mset_register_type (mset,
                                 TYPE1,
                                 type1_disp,
                                 type1_equal,
                                 type1_dup,
                                 NULL);
  elem1 = malloc (sizeof (struct type1_t));
  fail_if (elem1 == NULL);
  elem1->i = 1;
  e1 = rec_mset_append (mset, type, (void *) elem1, MSET_ANY);
  fail_if (e1 == NULL);

  /* Try to get invalid elements.  */
  e1aux = rec_mset_get_at (mset, MSET_ANY, -10);
  fail_if (e1aux != NULL);
  e1aux = rec_mset_get_at (mset, MSET_ANY, 1000);
  fail_if (e1aux != NULL);

  rec_mset_destroy (mset);
}
END_TEST


/*
 * Test case creation function
 */
TCase *
test_rec_mset_get_at (void)
{
  TCase *tc = tcase_create ("rec_mset_get");
  tcase_add_test (tc, rec_mset_get_at_empty);
  tcase_add_test (tc, rec_mset_get_at_existing);
  tcase_add_test (tc, rec_mset_get_at_any);
  tcase_add_test (tc, rec_mset_get_at_invalid);

  return tc;
}

/* End of rec-mset-get-at.c */
