/* -*- mode: C -*-
 *
 *       File:         tsuite-rec-type.c
 *       Date:         Fri Oct 29 16:13:26 2010
 *
 *       GNU recutils - rec_type test suite
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

extern TCase *test_rec_type_new (void);
extern TCase *test_rec_type_destroy (void);
extern TCase *test_rec_type_descr_p (void);
extern TCase *test_rec_type_kind (void);
extern TCase *test_rec_type_kind_str (void);
extern TCase *test_rec_type_equal_p (void);
extern TCase *test_rec_type_check (void);
extern TCase *test_rec_type_name (void);
extern TCase *test_rec_type_set_name (void);

Suite *
tsuite_rec_type ()
{
  Suite *s;

  s = suite_create ("rec-type");
  suite_add_tcase (s, test_rec_type_new ());
  suite_add_tcase (s, test_rec_type_destroy ());
  suite_add_tcase (s, test_rec_type_descr_p ());
  suite_add_tcase (s, test_rec_type_kind ());
  suite_add_tcase (s, test_rec_type_kind_str ());
  suite_add_tcase (s, test_rec_type_equal_p ());
  suite_add_tcase (s, test_rec_type_check ());
  suite_add_tcase (s, test_rec_type_name ());
  suite_add_tcase (s, test_rec_type_set_name ());

  return s;
}

/* End of tsuite-rec-type.c */
