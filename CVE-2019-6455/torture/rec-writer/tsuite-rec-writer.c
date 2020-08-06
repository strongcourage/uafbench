/* -*- mode: C -*-
 *
 *       File:         tsuite-rec-writer.c
 *       Date:         Sun Nov 14 10:54:46 2010
 *
 *       GNU recutils - rec_writer test suite
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
#include <check.h>

extern TCase *test_rec_writer_new (void);
extern TCase *test_rec_writer_destroy (void);
extern TCase *test_rec_write_comment (void);
extern TCase *test_rec_write_field_name (void);
extern TCase *test_rec_write_field (void);
extern TCase *test_rec_write_record (void);
extern TCase *test_rec_write_rset (void);
extern TCase *test_rec_write_db (void);
extern TCase *test_rec_write_field_name_str (void);
extern TCase *test_rec_write_field_str (void);
extern TCase *test_rec_write_comment_str (void);

Suite *
tsuite_rec_writer ()
{
  Suite *s;

  s = suite_create ("rec-writer");
  suite_add_tcase (s, test_rec_writer_new ());
  suite_add_tcase (s, test_rec_writer_destroy ());
  suite_add_tcase (s, test_rec_write_comment ());
  suite_add_tcase (s, test_rec_write_field_name ());
  suite_add_tcase (s, test_rec_write_field ());
  suite_add_tcase (s, test_rec_write_record ());
  suite_add_tcase (s, test_rec_write_rset ());
  suite_add_tcase (s, test_rec_write_db ());
  suite_add_tcase (s, test_rec_write_field_name_str ());
  suite_add_tcase (s, test_rec_write_field_str ());
  suite_add_tcase (s, test_rec_write_comment_str ());

  return s;
}

/* End of tsuite-rec-writer.c */
