/* -*- mode: C -*-
 *
 *       File:         rec-parse-db.c
 *       Date:         Sat Nov 13 21:59:40 2010
 *
 *       GNU recutils - rec_parse_db unit tests.
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
 * Test: rec_parse_db_nominal
 * Unit: rec_parse_db
 * Description:
 * + Parse valid databases.
 */
START_TEST(rec_parse_db_nominal)
{
  rec_parser_t parser;
  rec_db_t db;
  char *str;

  str = "%rec: foo\n\nfoo: bar\n\n%rec: bar\n\nfoo: bar\n\nfoo: bar";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (!rec_parse_db (parser, &db));
  fail_if (rec_db_size (db) != 2);
  rec_db_destroy (db);
  rec_parser_destroy (parser);
}
END_TEST

/*-
 * Test: rec_parse_db_invalid
 * Unit: rec_parse_db
 * Description:
 * + Try to parse invalid databases.
 */
START_TEST(rec_parse_db_invalid)
{
  /* Nothing here, since the empty string is a valid (empty)
     database.  */
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parse_db (void)
{
  TCase *tc = tcase_create ("rec_parse_db");
  tcase_add_test (tc, rec_parse_db_nominal);
  tcase_add_test (tc, rec_parse_db_invalid);

  return tc;
}

/* End of rec-parse-db.c */
