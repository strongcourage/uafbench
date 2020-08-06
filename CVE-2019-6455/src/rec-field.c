/* -*- mode: C -*-
 *
 *       File:         rec-field.c
 *       Date:         Fri Feb 27 20:40:26 2009
 *
 *       GNU recutils - Fields
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
#include <stdio.h>

#include <rec.h>

/* Field Data Structure.
 *
 * A field is an association between a label and a value.
 */

struct rec_field_s
{
  /* The name and the value of a field are UTF-8 encoded strings.
     Thus, we use NULL-terminated strings to store them.  */

  char *name;
  char *value;

  /* Localization.  */

  char *source;
  size_t location;
  char *location_str;
  size_t char_location;
  char *char_location_str;

  /* Field marks.  */

  int mark;
};

/* Static functions defined below.  */

static void rec_field_init (rec_field_t field);

/*
 * Public functions.
 */

const char *
rec_field_name (rec_field_t field)
{
  return field->name;
}

bool
rec_field_set_name (rec_field_t field, const char *name)
{
  free (field->name);
  field->name = strdup (name);
  return (field->name != NULL);
}

const char *
rec_field_value (rec_field_t field)
{
  return field->value;
}

bool
rec_field_set_value (rec_field_t field,
                     const char *value)
{
  free (field->value);
  field->value = strdup (value);
  return (field->value != NULL);
}

rec_field_t
rec_field_new (const char *name,
               const char *value)
{
  rec_field_t field;

  field = malloc (sizeof (struct rec_field_s));

  if (field != NULL)
    {
      rec_field_init (field);

      if (!rec_field_set_name (field, name))
        {
          /* Out of memory.  */
          rec_field_destroy (field);
          return NULL;
        }

      if (!rec_field_set_value (field, value))
        {
          /* Out of memory.  */
          rec_field_destroy (field);
          return NULL;
        }
    }
  
  return field;
}

rec_field_t
rec_field_dup (rec_field_t field)
{
  rec_field_t new_field;

  new_field = rec_field_new (rec_field_name (field),
                             rec_field_value (field));
  if (new_field)
    {
      new_field->location = field->location;
      new_field->char_location = field->char_location;
      new_field->mark = field->mark;

      if (field->source)
        {
          new_field->source = strdup (field->source);
          if (!new_field->source)
            {
              /* Out of memory.  */
              rec_field_destroy (new_field);
              return NULL;
            }
        }

      if (field->location_str)
        {
          new_field->location_str = strdup (field->location_str);
          if (!new_field->location_str)
            {
              /* Out of memory.  */
              rec_field_destroy (new_field);
              return NULL;
            }
        }

      if (field->char_location_str)
        {
          new_field->char_location_str = strdup (field->char_location_str);
          if (!new_field->char_location_str)
            {
              /* Out of memory.  */
              rec_field_destroy (new_field);
              return NULL;
            }
        }
    }

  return new_field;
}

bool
rec_field_equal_p (rec_field_t field1,
                   rec_field_t field2)
{
  return (strcmp (field1->name, field2->name) == 0);
}

void
rec_field_destroy (rec_field_t field)
{
  if (field)
    {
      free (field->name);
      free (field->value);
      free (field->source);
      free (field->location_str);
      free (field->char_location_str);
      free (field);
    }
}

rec_comment_t
rec_field_to_comment (rec_field_t field)
{
  rec_comment_t res;
  char *comment_str;
  
  comment_str = rec_write_field_str (field,
                                     REC_WRITER_NORMAL);
  if (!comment_str)
    {
      return NULL;
    }

  /* If the last character of the comment string is a newline, remove
     it.  */

  if (comment_str[strlen (comment_str) - 1] == '\n')
    {
      comment_str[strlen (comment_str) - 1] = '\0';
    }

  res = rec_comment_new (comment_str);
  free (comment_str);
  
  return res;
}

const char *
rec_field_source (rec_field_t field)
{
  return field->source;
}

bool
rec_field_set_source (rec_field_t field,
                      const char *source)
{
  free (field->source);
  field->source = strdup (source);
  return (field->source != NULL);
}

size_t
rec_field_location (rec_field_t field)
{
  return field->location;
}

bool
rec_field_set_location (rec_field_t field,
                        size_t location)
{
  field->location = location;
  free (field->location_str);
  return (asprintf (&(field->location_str), "%zu", field->location)
          != -1);
}

const char *
rec_field_location_str (rec_field_t field)
{
  char *res;

  if (field->location_str)
    {
      res = field->location_str;
    }
  else
    {
      res = "";
    }

  return res;
}

size_t
rec_field_char_location (rec_field_t field)
{
  return field->char_location;
}

bool
rec_field_set_char_location (rec_field_t field,
                             size_t location)
{
  field->char_location = location;
  free (field->char_location_str);
  return (asprintf (&(field->char_location_str), "%zu", field->char_location)
          != -1);
}

const char *
rec_field_char_location_str (rec_field_t field)
{
  char *res;

  if (field->char_location_str)
    {
      res = field->char_location_str;
    }
  else
    {
      res = "";
    }
  
  return res;
}

void
rec_field_set_mark (rec_field_t field, int mark)
{
  field->mark = mark;
}

int
rec_field_mark (rec_field_t field)
{
  return field->mark;
}

/*
 * Private functions.
 */

static void
rec_field_init (rec_field_t field)
{
  /* Initialize the field structure so it can be safely passed to
     rec_field_destroy even if its contents are not completely
     initialized with real values.  */

  memset (field, 0 /* NULL */, sizeof (struct rec_field_s));
}

/* End of rec-field.c */
