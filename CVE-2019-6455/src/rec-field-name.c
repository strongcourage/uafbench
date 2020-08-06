/* -*- mode: C -*-
 *
 *       File:         rec-field-name.c
 *       Date:         Fri Dec 25 17:27:05 2009
 *
 *       GNU recutils - Field names.
 *
 */

/* Copyright (C) 2009-2019 Jose E. Marchesi */

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
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <gettext.h>
#define _(str) dgettext (PACKAGE, str)

#include <rec-utils.h>
#include <rec.h>

/* The following global variable contains field names which are either
   standard, such as the special field names, or often used in the
   library.
   
   Note that the array is indexed using the rec_std_field_e enumerated
   values defined in rec.h, so the order is important.  */

static const char *fnames[] =
  { 
    "%auto",
    "%confidential",
    "%key",
    "%mandatory",
    "%prohibit",
    "%rec",
    "%size",
    "%sort",
    "%type",
    "%typedef",
    "%unique",
    "%constraint",
    "%allowed"
  };

const char *
rec_std_field_name (enum rec_std_field_e std_field)
{
  return *(fnames + std_field);
}

bool
rec_field_name_p (const char *str)
{
  return rec_match (str, "^" REC_FNAME_RE "$");
}

char *
rec_field_name_normalise (const char *str)
{
  char *normalised_name;
  int i;

  normalised_name = malloc (strlen (str) + 1);
  if (normalised_name)
    {
      memcpy (normalised_name, str, strlen (str));
      normalised_name[strlen(str)] = '\0';
      
      for (i = 0; i < strlen (normalised_name); i++)
        {
          if (!((isalnum (normalised_name[i]))
                || (normalised_name[i] == '_')
                || (normalised_name[i] == '-')
                || (normalised_name[i] == '%')))
            {
              normalised_name[i] = '_';
            }
        }
    }

  /* Check that the normalisation produced a proper field name.  */

  if (normalised_name)
    {
      if (!rec_field_name_p (normalised_name))
        {
          free (normalised_name);
          normalised_name = NULL;
        }
    }

  return normalised_name;
}

bool
rec_field_name_equal_p (const char *name1,
                        const char *name2)
{
  /* TODO: 'foo' and 'foo:' denote the same field name.  */
  return (strcmp (name1, name2) == 0);
}

/* End of rec-field-name.c */
