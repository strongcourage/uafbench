/* -*- mode: C -*-
 *
 *       File:         tsuite-rec-sex.c
 *       Date:         Mon Nov 15 15:04:12 2010
 *
 *       GNU recutils - rec_sex test suite.
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

extern TCase *test_rec_sex_new (void);
extern TCase *test_rec_sex_destroy (void);
extern TCase *test_rec_sex_compile (void);
extern TCase *test_rec_sex_eval (void);

Suite *
tsuite_rec_sex ()
{
  Suite *s;

  s = suite_create ("rec-sex");
  suite_add_tcase (s, test_rec_sex_new ());
  suite_add_tcase (s, test_rec_sex_destroy ());
  suite_add_tcase (s, test_rec_sex_compile ());
  suite_add_tcase (s, test_rec_sex_eval ());

  return s;
}

/* End of tsuite-rec-sex.c */
