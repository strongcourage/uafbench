/* -*- mode: C -*-
 *
 *       File:         rec-field-to-comment.c
 *       Date:         Fri Nov 12 15:38:17 2010
 *
 *       GNU recutils - rec_field_to_comment unit tests.
 *
 */

/* Copyright (C) 2009-2015 Jose E. Marchesi */

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
 * Test: rec_field_to_comment_nominal
 * Unit: rec_field_to_comment
 * Description:
 * + Convert fields into comments.
 */
START_TEST(rec_field_to_comment_nominal)
{
  rec_field_t field;
  rec_comment_t comment;

  field = rec_field_new ("foo", "");
  fail_if (field == NULL);
  comment = rec_field_to_comment (field);
  fail_if (strcmp (rec_comment_text (comment), "foo:") != 0);
  rec_comment_destroy (comment);
  rec_field_destroy (field);

  field = rec_field_new ("foo", "value");
  fail_if (field == NULL);
  comment = rec_field_to_comment (field);
  fail_if (strcmp (rec_comment_text (comment), "foo: value") != 0);
  rec_comment_destroy (comment);
  rec_field_destroy (field);

  field = rec_field_new ("foo", "line1\nline2");
  fail_if (field == NULL);
  comment = rec_field_to_comment (field);
  fail_if (strcmp (rec_comment_text (comment), "foo: line1\n+ line2") != 0);
  rec_comment_destroy (comment);
  rec_field_destroy (field);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_to_comment (void)
{
  TCase *tc = tcase_create ("rc_field_to_comment");
  tcase_add_test (tc, rec_field_to_comment_nominal);

  return tc;
}


/* rec-field-to-comment.c */
