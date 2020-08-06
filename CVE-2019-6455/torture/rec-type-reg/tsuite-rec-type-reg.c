/* -*- mode: C -*- Time-stamp: "2015-04-27 20:12:44 jemarch"
 *
 *       File:         tsuite-rec-type-reg.c
 *       Date:         Fri Oct 29 20:58:54 2010
 *
 *       GNU recutils - rec_type_reg test suite
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

extern TCase *test_rec_type_reg_new (void);
extern TCase *test_rec_type_reg_destroy (void);
extern TCase *test_rec_type_reg_get (void);

Suite *
tsuite_rec_type_reg ()
{
  Suite *s;

  s = suite_create ("rec-type-reg");
  suite_add_tcase (s, test_rec_type_reg_new ());
  suite_add_tcase (s, test_rec_type_reg_destroy ());
  suite_add_tcase (s, test_rec_type_reg_get ());

  return s;
}

/* End of tsuite-rec-type-reg.c */
