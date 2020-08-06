/* -*- mode: C -*-
 *
 *       File:         rec-comment-new.c
 *       Date:         Fri Oct 29 11:22:47 2010
 *
 *       GNU recutils - rec_comment_new unit tests
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
 * Test: rec_comment_new_nominal
 * Unit: rec_comment_new
 * Description:
 * + Create a new comment from a string.
 * +
 * + 1. rec_comment_new shall return a non-NULL pointer.
 */
START_TEST(rec_comment_new_nominal)
{
  rec_comment_t comment;

  comment = rec_comment_new ("foo");
  fail_if (comment == NULL);

  rec_comment_destroy (comment);
}
END_TEST

/*-
 * Test: rec_comment_new_null
 * Unit: rec_comment_new
 * Description:
 * + Create a new comment from a NULL.
 * +
 * + 1. rec_comment_new shall return NULL.
 */
START_TEST(rec_comment_new_null)
{
  rec_comment_t comment;

  comment = rec_comment_new (NULL);
  fail_if (comment != NULL);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_comment_new (void)
{
  TCase *tc = tcase_create ("rec_comment_new");
  tcase_add_test (tc, rec_comment_new_nominal);
  tcase_add_test (tc, rec_comment_new_null);

  return tc;
}

/* End of rec-comment-new.c */
