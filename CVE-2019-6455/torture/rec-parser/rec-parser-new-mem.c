/* -*- mode: C -*-
 *
 *       File:         rec-parser-new-mem.c
 *       Date:         Fri May 25 11:14:05 2012
 *
 *       GNU recutils - rec_parser_new_mem unit tests.
 *
 */

/* Copyright (C) 2010-2015 Jose E. Marchesi */
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

/* Based on rec_parse_rset unit tests. */

/*-
 * Test: rec_parser_new_mem_nominal
 * Unit: rec_parser_new_mem
 * Description:
 * + Parse a valid truncated record set.
 */
START_TEST(rec_parser_new_mem_nominal)
{
  rec_parser_t parser;
  rec_rset_t rset;
  char *str;
  char *short_str;

  str = "foo1: bar1\n\nfoo2: bar2\n\nfoo3: bar3";
  short_str = "foo1: bar1\n\nfoo2: bar2";
  parser = rec_parser_new_mem (str, strlen(short_str), "dummy");
  fail_if (!rec_parse_rset (parser, &rset));
  fail_if (rec_rset_num_records (rset) != 2);
  rec_rset_destroy (rset);
  rec_parser_destroy (parser);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parser_new_mem (void)
{
  TCase *tc = tcase_create ("rec_parser_new_mem");
  tcase_add_test (tc, rec_parser_new_mem_nominal);

  return tc;
}

/* End of rec-parser-new-mem.c */
