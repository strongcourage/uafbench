/* -*- mode: C -*-
 *
 *       File:         rec-parse-rset.c
 *       Date:         Sat Nov 13 21:30:44 2010
 *
 *       GNU recutils - rec_parse_rset unit tests.
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
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_parse_rset_nominal
 * Unit: rec_parse_rset
 * Description:
 * + Parse valid record sets.
 */
START_TEST(rec_parse_rset_nominal)
{
  rec_parser_t parser;
  rec_rset_t rset;
  char *str;

  str = "foo1: bar1\n\nfoo2: bar2\n\nfoo3: bar3";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (!rec_parse_rset (parser, &rset));
  fail_if (rec_rset_num_records (rset) != 3);
  rec_rset_destroy (rset);
  rec_parser_destroy (parser);

  str = "%rec: foo\n\nfoo1: bar1\n\nfoo2: bar2\n\nfoo3: bar3";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (!rec_parse_rset (parser, &rset));
  fail_if (rec_rset_num_records (rset) != 3);
  rec_rset_destroy (rset);
  rec_parser_destroy (parser);

  str = "foo1: bar1\n\n#foo2: bar2\n\nfoo3: bar3";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (!rec_parse_rset (parser, &rset));
  fail_if (rec_rset_num_elems (rset) != 3);
  fail_if (rec_rset_num_comments (rset) != 1);
  fail_if (rec_rset_num_records (rset) != 2);
  rec_rset_destroy (rset);
  rec_parser_destroy (parser);
}
END_TEST

/*-
 * Test: rec_parse_rset_invalid
 * Unit: rec_parse_rset
 * Description:
 * + Try to parse invalid record sets.
 */
START_TEST(rec_parse_rset_invalid)
{
  rec_parser_t parser;
  rec_rset_t rset;
  char *str;

  str = " ";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_rset (parser, &rset));
  rec_parser_destroy (parser);

  /* A record set shall have at least one record.  */
  str = "#foo1: bar1\n\n#foo2: bar2\n\n#foo3: bar3";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (rec_parse_rset (parser, &rset));
  rec_parser_destroy (parser);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parse_rset (void)
{
  TCase *tc = tcase_create ("rec_parse_rset");
  tcase_add_test (tc, rec_parse_rset_nominal);
  tcase_add_test (tc, rec_parse_rset_invalid);

  return tc;
}

/* End of rec-parse-rset.c */
