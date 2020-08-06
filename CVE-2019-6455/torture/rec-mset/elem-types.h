/* -*- mode: C -*-
 *
 *       File:         elem-types.h
 *       Date:         Thu Oct 28 20:01:31 2010
 *
 *       GNU recutils - Element types for the rec_mset tests.
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
#include <stdlib.h>

/* mset types used by the tests in this file:
 * 
 * TYPE1 and TYPE2
 */

#define TYPE1 "TYPE1"

struct type1_t
{
  int i;
};

static void
type1_disp (void *data)
{
  struct type1_t *s;

  s = (struct type1_t *) data;
  free (s);
}

static bool
type1_equal (void *data1,
             void *data2)
{
  struct type1_t *s1;
  struct type1_t *s2;

  s1 = (struct type1_t *) data1;
  s2 = (struct type1_t *) data2;

  return (s1->i == s2->i);
}

static void *
type1_dup (void *data)
{
  struct type1_t *s;
  struct type1_t *s_copy;

  s = (struct type1_t *) data;
  s_copy = malloc (sizeof (struct type1_t));
  if (s_copy)
    {
      s_copy->i = s->i;
    }

  return (void *) s_copy;
}

#define TYPE2 "TYPE2"

struct type2_t
{
  char c;
};

static void
type2_disp (void *data)
{
  struct type2_t *s;

  s = (struct type2_t *) data;
  free (s);
}

static bool type2_equal (void *data1,
                         void *data2)
{
  struct type2_t *s1;
  struct type2_t *s2;

  s1 = (struct type2_t *) data1;
  s2 = (struct type2_t *) data2;

  return (s1->c == s2->c);
}

static void *
type2_dup (void *data)
{
  struct type2_t *s;
  struct type2_t *s_copy;

  s = (struct type2_t *) data;
  s_copy = malloc (sizeof (struct type2_t));
  if (s_copy)
    {
      s_copy->c = s->c;
    }

  return (void *) s_copy;
}

/* End of elem-types.h */
