/* -*- mode: C -*-
 *
 *       File:         rec-write-record.c
 *       Date:         Sun Nov 14 13:27:25 2010
 *
 *       GNU recutils - rec_write_record unit tests.
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
#include <stdlib.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_write_record_nominal
 * Unit: rec_write_record
 * Description:
 * + Write records.
 */
START_TEST(rec_write_record_nominal)
{
  rec_writer_t writer;
  rec_record_t record;
  rec_field_t field;
  rec_comment_t comment;
  char *str;
  size_t str_size;

  record = rec_record_new ();
  fail_if (record == NULL);
  field = rec_field_new ("foo1", "value1");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  comment = rec_comment_new ("comment");
  fail_if (comment == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_COMMENT, (void *) comment, MSET_ANY) == NULL);
  field = rec_field_new ("foo2", "value2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  rec_writer_set_mode (writer, REC_WRITER_NORMAL);
  fail_if (!rec_write_record (writer, record));
  rec_record_destroy (record);
  rec_writer_destroy (writer);
  fail_if (strcmp (str,
                   "foo1: value1\n#comment\nfoo2: value2") != 0);
  free (str);
}
END_TEST

/*-
 * Test: rec_write_record_skip_comments
 * Unit: rec_write_record
 * Description:
 * + Write records.
 */
START_TEST(rec_write_record_skip_comments)
{
  rec_writer_t writer;
  rec_record_t record;
  rec_field_t field;
  rec_comment_t comment, comment2;
  char *str;
  size_t str_size;

  record = rec_record_new ();
  fail_if (record == NULL);
  field = rec_field_new ("foo1", "value1");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  comment = rec_comment_new ("comment");
  fail_if (comment == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_COMMENT, (void *) comment, MSET_ANY) == NULL);
  field = rec_field_new ("foo2", "value2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  comment2 = rec_comment_new ("comment2");
  fail_if (comment2 == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_COMMENT, (void *) comment2, MSET_ANY) == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  rec_writer_set_mode (writer, REC_WRITER_NORMAL);
  rec_writer_set_skip_comments (writer, true);
  fail_if (!rec_write_record (writer, record));
  rec_record_destroy (record);
  rec_writer_destroy (writer);
  fail_if (strcmp (str,
                   "foo1: value1\nfoo2: value2") != 0);
  free (str);
}
END_TEST

/*-
 * Test: rec_write_record_sexp
 * Unit: rec_write_record
 * Description:
 * + Write records in sexps.
 */
START_TEST(rec_write_record_sexp)
{
  rec_writer_t writer;
  rec_record_t record;
  rec_field_t field;
  rec_comment_t comment;
  char *str;
  size_t str_size;

  record = rec_record_new ();
  fail_if (record == NULL);
  field = rec_field_new ("foo1", "value1");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  comment = rec_comment_new ("comment");
  fail_if (comment == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_COMMENT, (void *) comment, MSET_ANY) == NULL);
  field = rec_field_new ("foo2", "value2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  rec_writer_set_mode (writer, REC_WRITER_SEXP);
  fail_if (!rec_write_record (writer, record));
  rec_record_destroy (record);
  rec_writer_destroy (writer);
  fail_if (strcmp (str,
                   "(record  (\n(field  \"foo1\" \"value1\")\n(comment \"comment\")\n(field  \"foo2\" \"value2\")))") != 0);
  free (str);
}
END_TEST

/*-
 * Test: rec_write_record_values
 * Unit: rec_write_record
 * Description:
 * + Write records using the
 * + REC_WRITER_VALUES mode.
 */
START_TEST(rec_write_record_values)
{
  rec_writer_t writer;
  rec_record_t record;
  rec_field_t field;
  rec_comment_t comment;
  char *str;
  size_t str_size;

  record = rec_record_new ();
  fail_if (record == NULL);
  field = rec_field_new ("foo1", "value1");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  comment = rec_comment_new ("comment");
  fail_if (comment == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_COMMENT, (void *) comment, MSET_ANY) == NULL);
  field = rec_field_new ("foo2", "value2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  rec_writer_set_mode (writer, REC_WRITER_VALUES);
  fail_if (!rec_write_record (writer, record));
  rec_record_destroy (record);
  rec_writer_destroy (writer);
  fail_if (strcmp (str, "value1\nvalue2") != 0);
  free (str);
}
END_TEST

/*-
 * Test: rec_write_record_values_row
 * Unit: rec_write_record
 * Description:
 * + Write records using the
 * + REC_WRITER_VALUES_ROW mode.
 */
START_TEST(rec_write_record_values_row)
{
  rec_writer_t writer;
  rec_record_t record;
  rec_field_t field;
  rec_comment_t comment;
  char *str;
  size_t str_size;

  record = rec_record_new ();
  fail_if (record == NULL);
  field = rec_field_new ("foo1", "value1");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  comment = rec_comment_new ("comment");
  fail_if (comment == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_COMMENT, (void *) comment, MSET_ANY) == NULL);
  field = rec_field_new ("foo2", "value2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset(record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  rec_writer_set_mode (writer, REC_WRITER_VALUES_ROW);
  fail_if (!rec_write_record (writer, record));
  rec_record_destroy (record);
  rec_writer_destroy (writer);
  fail_if (strcmp (str, "value1 value2") != 0);
  free (str);
}
END_TEST


/*
 * Test creation function
 */
TCase *
test_rec_write_record (void)
{
  TCase *tc = tcase_create ("rec_write_record");
  tcase_add_test (tc, rec_write_record_nominal);
  tcase_add_test (tc, rec_write_record_skip_comments);
  tcase_add_test (tc, rec_write_record_sexp);
  tcase_add_test (tc, rec_write_record_values);
  tcase_add_test (tc, rec_write_record_values_row);

  return tc;
}

/* End of rec-write-record.c */
