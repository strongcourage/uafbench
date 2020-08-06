/* -*- mode: C -*-
 *
 *       File:         rec-comment-text.c
 *       Date:         Fri Oct 29 11:48:45 2010
 *
 *       GNU recutils - rec_comment_text unit tests
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
 * Test: rec_comment_text_nominal
 * Unit: rec_comment_text
 * Description:
 * + Get the contents of a comment.
 * +
 * + 1. The function shall return the right contents.
 */
START_TEST(rec_comment_text_nominal)
{
  rec_comment_t comment;

  comment = rec_comment_new ("abc");
  fail_if (comment == NULL);
  fail_if (strcmp ("abc",
                   rec_comment_text (comment)) != 0);
  
  rec_comment_destroy (comment);
}
END_TEST

/*-
 * Test: rec_comment_text_empty
 * Unit: rec_comment_text
 * Description:
 * + Get the contents of an empty comment.
 * +
 * + 1. The function shall return the empty string.
 */
START_TEST(rec_comment_text_empty)
{
  rec_comment_t comment;

  comment = rec_comment_new ("");
  fail_if (comment == NULL);
  fail_if (strcmp ("",
                   rec_comment_text (comment)) != 0);

  rec_comment_destroy (comment);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_comment_text (void)
{
  TCase *tc = tcase_create ("rec-comment-text");
  tcase_add_test (tc, rec_comment_text_nominal);
  tcase_add_test (tc, rec_comment_text_empty);

  return tc;
}

/* End of rec-comment-text.c */
