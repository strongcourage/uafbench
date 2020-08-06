/* -*- mode: C -*-
 *
 *       File:         rec-comment-equal-p.c
 *       Date:         Fri Oct 29 11:58:25 2010
 *
 *       GNU recutils - rec_comment_equal_p unit tests
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
 * Test: rec_comment_equal_p_nominal
 * Unit: rec_comment_equal_p
 * Description:
 * + Compare comments.
 * +
 * + 1. The comparisons shall be correct.
 */
START_TEST(rec_comment_equal_p_nominal)
{
  rec_comment_t comment1;
  rec_comment_t comment2;
  rec_comment_t comment3;
  rec_comment_t comment_empty;

  comment1 = rec_comment_new ("abc");
  fail_if (comment1 == NULL);
  
  comment2 = rec_comment_new ("xyz");
  fail_if (comment2 == NULL);

  comment3 = rec_comment_new ("abc"); /* Same than comment1 */
  fail_if (comment3 == NULL);

  comment_empty = rec_comment_new ("");
  fail_if (comment_empty == NULL);

  fail_if (rec_comment_equal_p (comment1, comment2));
  fail_if (rec_comment_equal_p (comment1, comment_empty));
  fail_if (rec_comment_equal_p (comment2, comment_empty));
  fail_if (!rec_comment_equal_p (comment1, comment1));
  fail_if (!rec_comment_equal_p (comment_empty, comment_empty));
  fail_if (!rec_comment_equal_p (comment1, comment3));
           
  rec_comment_destroy (comment1);
  rec_comment_destroy (comment2);
  rec_comment_destroy (comment3);
  rec_comment_destroy (comment_empty);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_comment_equal_p (void)
{
  TCase *tc = tcase_create ("rec-comment-equal-p");
  tcase_add_test (tc, rec_comment_equal_p_nominal);

  return tc;
}

/* End of rec-comment-equal-p.c */
