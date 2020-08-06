/* -*- mode: C -*-
 *
 *       File:         tsuite-rec-parser.c
 *       Date:         Sat Nov 13 15:20:44 2010
 *
 *       GNU recutils - rec_parser test suite
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
#include <check.h>

extern TCase *test_rec_parser_new (void);
extern TCase *test_rec_parser_new_str (void);
extern TCase *test_rec_parser_new_mem (void);
extern TCase *test_rec_parser_destroy (void);
extern TCase *test_rec_parse_field_name_str (void);
extern TCase *test_rec_parse_field_name (void);
extern TCase *test_rec_parse_field (void);
extern TCase *test_rec_parse_record (void);
extern TCase *test_rec_parse_record_str (void);
extern TCase *test_rec_parse_rset (void);
extern TCase *test_rec_parse_db (void);
extern TCase *test_rec_parser_eof (void);
extern TCase *test_rec_parser_error (void);
extern TCase *test_rec_parser_reset (void);
extern TCase *test_rec_parser_perror (void);
extern TCase *test_rec_parser_seek_mem (void);

Suite *
tsuite_rec_parser ()
{
  Suite *s;

  s = suite_create ("rec-parser");
  suite_add_tcase (s, test_rec_parser_new ());
  suite_add_tcase (s, test_rec_parser_new_str ());
  suite_add_tcase (s, test_rec_parser_new_mem ());
  suite_add_tcase (s, test_rec_parser_destroy ());
  suite_add_tcase (s, test_rec_parse_field_name_str ());
  suite_add_tcase (s, test_rec_parse_field_name ());
  suite_add_tcase (s, test_rec_parse_field ());
  suite_add_tcase (s, test_rec_parse_record ());
  suite_add_tcase (s, test_rec_parse_record_str ());
  suite_add_tcase (s, test_rec_parse_rset ());
  suite_add_tcase (s, test_rec_parse_db ());
  suite_add_tcase (s, test_rec_parser_eof ());
  suite_add_tcase (s, test_rec_parser_error ());
  suite_add_tcase (s, test_rec_parser_reset ());
  suite_add_tcase (s, test_rec_parser_perror ());
  suite_add_tcase (s, test_rec_parser_seek_mem ());

  return s;
}


/* End of tsuite-rec-parser.c */
