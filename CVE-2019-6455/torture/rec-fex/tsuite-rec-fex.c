/* -*- mode: C -*-
 *
 *       File:         tsuite-rec-fex.c
 *       Date:         Tue Nov  9 14:02:54 2010
 *
 *       GNU recutils - rec_fex test suite.
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

extern TCase *test_rec_fex_new (void);
extern TCase *test_rec_fex_destroy (void);
extern TCase *test_rec_fex_check (void);
extern TCase *test_rec_fex_sort (void);
extern TCase *test_rec_fex_size (void);
extern TCase *test_rec_fex_get (void);
extern TCase *test_rec_fex_elem_field_name (void);
extern TCase *test_rec_fex_elem_rewrite_to (void);
extern TCase *test_rec_fex_elem_min (void);
extern TCase *test_rec_fex_elem_max (void);
extern TCase *test_rec_fex_str (void);

Suite *
tsuite_rec_fex ()
{
  Suite *s;

  s = suite_create ("rec-fex");
  suite_add_tcase (s, test_rec_fex_new ());
  suite_add_tcase (s, test_rec_fex_destroy ());
  suite_add_tcase (s, test_rec_fex_check ());
  suite_add_tcase (s, test_rec_fex_sort ());
  suite_add_tcase (s, test_rec_fex_size ());
  suite_add_tcase (s, test_rec_fex_get ());
  suite_add_tcase (s, test_rec_fex_elem_field_name ());
  suite_add_tcase (s, test_rec_fex_elem_rewrite_to ());
  suite_add_tcase (s, test_rec_fex_elem_min ());
  suite_add_tcase (s, test_rec_fex_elem_max ());
  suite_add_tcase (s, test_rec_fex_str ());

  return s;
}

/* End of tsuite-rec-fex.c */
