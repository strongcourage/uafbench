/* -*- mode: C -*-
 *
 *       File:         tsuite-rec-record.c
 *       Date:         Thu Mar  5 23:56:38 2009
 *
 *       GNU recutils - rec_record test suite
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

/* extern TCase *test_rec_record_new (void);
extern TCase *test_rec_record_size (void);
extern TCase *test_rec_record_field_p (void);
extern TCase *test_rec_record_insert_field (void);
extern TCase *test_rec_record_remove_field (void); */

Suite *
tsuite_rec_record ()
{
  Suite *s;

  s = suite_create ("rec-record");
  /*  suite_add_tcase (s, test_rec_record_new ());
  suite_add_tcase (s, test_rec_record_size ());
  suite_add_tcase (s, test_rec_record_field_p ());
  suite_add_tcase (s, test_rec_record_insert_field ());
  suite_add_tcase (s, test_rec_record_remove_field ()); */

  return s;
}


/* End of tsuite-rec-record.c */
