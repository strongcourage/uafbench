/* -*- mode: C -*-
 *
 *       File:         rec-comment-dup.c
 *       Date:         Fri Oct 29 11:31:22 2010
 *
 *       GNU recutils - rec_comment_dup unit tests
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
 * Test: rec_comment_dup_nominal
 * Unit: rec_comment_dup
 * Description:
 * + Dup a properly initialized comment.
 * +
 * + 1. The call to rec_comment_dup shall return a
 * +    non-null pointer different that the original.
 * + 2. The contents of the copy shall be the same
 * +    than the contents of the original.
 */
START_TEST(rec_comment_dup_nominal)
{
  rec_comment_t comment;
  rec_comment_t copy;

  comment = rec_comment_new ("abc");
  fail_if (comment == NULL);

  copy = rec_comment_dup (comment);
  fail_if (copy == NULL);
  fail_if (copy == comment);
  fail_if (strcmp (rec_comment_text (copy), "abc") != 0);
  
  rec_comment_destroy (comment);
  rec_comment_destroy (copy);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_comment_dup (void)
{
  TCase *tc = tcase_create ("rec-comment-dup");
  tcase_add_test (tc, rec_comment_dup_nominal);

  return tc;
}

/* End of rec-comment-dup.c */
