/* -*- mode: C -*-
 *
 *       File:         rec-parser-seek-mem.c
 *       Date:         Fri May 25 15:13:53 2012
 *
 *       GNU recutils - rec_parser_seek unit tests using memory buffers.
 *
 */

/* Copyright (C) 2012-2015 Michał Masłowski */

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
 * Test: rec_parser_seek_mem_tell
 * Unit: rec_parser_seek_mem
 * Description:
 * + Check that tell gives the same position as seeked to.
 */
START_TEST(rec_parser_seek_mem_tell)
{
  rec_parser_t parser;
  char *str;
  size_t position;

  str = "foo1: bar1\n\nfoo2: bar2\n\nfoo3: bar3";
  position = strlen ("foo1: bar\n\n");
  parser = rec_parser_new_mem (str, strlen (str), "dummy");
  fail_if (!parser);
  fail_if (rec_parser_tell (parser) != 0);
  fail_if (!rec_parser_seek (parser, 3, position));
  fail_if (rec_parser_tell (parser) != position);
  rec_parser_destroy (parser);
}
END_TEST

/*-
 * Test: rec_parser_seek_mem_skip
 * Unit: rec_parser_seek_mem
 * Description:
 * + Check that seek might skip a part of a record.
 */
START_TEST(rec_parser_seek_mem_skip)
{
  rec_parser_t parser;
  rec_record_t record;
  char *str;
  size_t position;

  str = "foo1: bar1\nfoo2: bar2\n\nfoo3: bar3";
  position = strlen ("foo1: bar1\n");
  parser = rec_parser_new_mem (str, strlen (str), "dummy");
  fail_if (!parser);
  fail_if (!rec_parser_seek (parser, 2, position));
  fail_if (!rec_parse_record (parser, &record));
  fail_if (rec_record_num_fields (record) != 1);
  fail_if (rec_record_location (record) != 2);
  rec_record_destroy (record);
  rec_parser_destroy (parser);
}
END_TEST

/*-
 * Test: rec_parser_seek_mem_fail
 * Unit: rec_parser_seek_mem
 * Description:
 * + Check that seek outside the file fails.
 */
START_TEST(rec_parser_seek_mem_fail)
{
  rec_parser_t parser;
  char *str;
  size_t position;

  str = "foo1: bar1\n\nfoo2: bar2\n\nfoo3: bar3";
  position = strlen (str) + 100;
  parser = rec_parser_new_mem (str, strlen (str), "dummy");
  fail_if (!parser);
  fail_if (rec_parser_seek (parser, 20, position));
  rec_parser_destroy (parser);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parser_seek_mem (void)
{
  TCase *tc = tcase_create ("rec_parser_seek_mem");
  tcase_add_test (tc, rec_parser_seek_mem_tell);
  tcase_add_test (tc, rec_parser_seek_mem_skip);
  tcase_add_test (tc, rec_parser_seek_mem_fail);

  return tc;
}

/* End of rec-parser-seek-mem.c */
