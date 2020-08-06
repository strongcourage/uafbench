/* -*- mode: C -*-
 *
 *       File:         rec-record.c
 *       Date:         Thu Mar  5 17:11:41 2009
 *
 *       GNU recutils - Records
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

#include <rec.h>
#include <rec-utils.h>

/*
 * Record data structures.
 */

struct rec_record_s
{
  /* Container pointer.  The semantics of this pointer depends on the
     user.  */

  void *container;

  /* type ids for the elements stored in the mset.  */

  int field_type;
  int comment_type;

  /* Localization.  */

  char *source;
  size_t location;
  char *location_str;
  size_t char_location;
  char *char_location_str;

  /* The internal multi-set storing the data.  */

  rec_mset_t mset;
};

/* Static functions implemented below.  */

static void rec_record_init (rec_record_t record);
static void rec_record_field_disp_fn (void *data);
static bool rec_record_field_equal_fn (void *data1, void *data2);
static void *rec_record_field_dup_fn (void *data);
static void rec_record_comment_disp_fn (void *data);
static bool rec_record_comment_equal_fn (void *data1, void *data2);
static void *rec_record_comment_dup_fn (void *data);

/*
 * Public functions.
 */

rec_record_t
rec_record_new (void)
{
  rec_record_t record;

  record = malloc (sizeof (struct rec_record_s));

  if (record)
    {
      rec_record_init (record);

      /* The container pointer is initially NULL, until the client
         uses it for something else.  */
      record->container = NULL;

      /* Localization information is not used until the user
         explicitly sets it.  */

      record->source = NULL;
      record->location = 0;
      record->location_str = NULL;
      record->char_location = 0;
      record->char_location_str = NULL;

      /* Create the multi-set that will hold the elements of the
         record.  Note that the order in which the types are
         registered is significative.  If you change the order please
         update the MSET_FIELD and MSET_COMMENT constants in
         rec.h.  */

      record->mset = rec_mset_new ();
      if (record->mset)
        {
          record->field_type = rec_mset_register_type (record->mset,
                                                       "field",
                                                       rec_record_field_disp_fn,
                                                       rec_record_field_equal_fn,
                                                       rec_record_field_dup_fn,
                                                       NULL);

          record->comment_type = rec_mset_register_type (record->mset,
                                                         "comment",
                                                         rec_record_comment_disp_fn,
                                                         rec_record_comment_equal_fn,
                                                         rec_record_comment_dup_fn,
                                                         NULL);
        }
      else
        {
          /* Out of memory.  */

          rec_record_destroy (record);
          record = NULL;
        }
    }

  return record;
}

void
rec_record_destroy (rec_record_t record)
{
  if (record)
    {
      free (record->source);
      free (record->location_str);
      free (record->char_location_str);
      rec_mset_destroy (record->mset);
      free (record);
    }
}

rec_record_t
rec_record_dup (rec_record_t record)
{
  rec_record_t new;

  new = malloc (sizeof (struct rec_record_s));
  if (new)
    {
      rec_record_init (new);

      new->field_type = record->field_type;
      new->comment_type = record->comment_type;
      new->mset = rec_mset_dup (record->mset);
      if (!new->mset)
        {
          rec_record_destroy (new);
          return NULL;
        }

      new->source = NULL;
      if (record->source)
        {
          new->source = strdup (record->source);
          if (!new->source)
            {
              rec_record_destroy (new);
              return NULL;
            }
        }

      new->location = record->location;
      new->char_location = record->char_location;

      new->location_str = NULL;
      if (record->location_str)
        {
          new->location_str = strdup (record->location_str);
          if (!new->location_str)
            {
              rec_record_destroy (new);
              return NULL;
            }
        }

      new->char_location_str = NULL;
      if (record->char_location_str)
        {
          new->char_location_str = strdup (record->char_location_str);
          if (!new->char_location_str)
            {
              rec_record_destroy (new);
              return NULL;
            }
        }

      new->container = record->container;
    }

  return new;
}

bool
rec_record_subset_p (rec_record_t record1,
                     rec_record_t record2)
{
  bool result;
  rec_mset_iterator_t iter1;
  rec_mset_elem_t elem1;
  const void *data;

  result = true;

  /* Iterate on the elements stored in record1.  For each element,
     verify that an equal element is also stored in record2.  */

  iter1 = rec_mset_iterator (record1->mset);
  while (rec_mset_iterator_next (&iter1, MSET_ANY, &data, &elem1))
    {
      bool elem_found = false;
      rec_mset_elem_t elem2;
      rec_mset_iterator_t iter2 = rec_mset_iterator (record2->mset);

      while (rec_mset_iterator_next (&iter2, MSET_ANY, &data, &elem2))
        {
          if (rec_mset_elem_equal_p (elem1, elem2))
            {
              elem_found = true;
              break;
            }
        }

      if (!elem_found)
        {
          result = false;
          break;
        }

      rec_mset_iterator_free (&iter2);
      
    }

  rec_mset_iterator_free (&iter1);
  
  return result;
}

bool
rec_record_equal_p (rec_record_t record1,
                    rec_record_t record2)
{
  return ((rec_record_subset_p (record1, record2)) &&
          (rec_record_subset_p (record2, record1)));
}

rec_mset_t
rec_record_mset (rec_record_t record)
{
  return record->mset;
}

size_t
rec_record_num_elems (rec_record_t record)
{
  return rec_mset_count (record->mset, MSET_ANY);
}

size_t
rec_record_num_fields (rec_record_t record)
{
  return rec_mset_count (record->mset, record->field_type);
}

size_t
rec_record_num_comments (rec_record_t record)
{
  return rec_mset_count (record->mset, record->comment_type);
}

size_t
rec_record_get_field_index (rec_record_t record,
                            rec_field_t field)
{
  size_t res = 0;
  rec_mset_iterator_t iter;
  rec_field_t list_field;

  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &list_field, NULL))
    {
      if (field == list_field)
        {
          break;
        }

      res++;
    }

  rec_mset_iterator_free (&iter);

  return res;
}

bool
rec_record_field_p (rec_record_t record,
                    const char *field_name)
{
  return (rec_record_get_num_fields_by_name (record, field_name) > 0);
}

size_t
rec_record_get_num_fields_by_name (rec_record_t record,
                                   const char *field_name)
{
  rec_mset_iterator_t iter;
  rec_field_t field;
  int num_fields = 0;

  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
    {
      if (rec_field_name_equal_p (rec_field_name (field), field_name))
        {
          num_fields++;
        }
    }
  rec_mset_iterator_free (&iter);

  return num_fields;
}

rec_field_t
rec_record_get_field_by_name (rec_record_t record,
                              const char *field_name,
                              size_t n)
{
  size_t num_fields = 0;
  rec_field_t field  = NULL;
  rec_field_t result = NULL;
  rec_mset_iterator_t iter;

  
  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
    {
      if (rec_field_name_equal_p (rec_field_name (field), field_name))
        {
          if (n == num_fields)
            {
              result = field;
              break;
            }

          num_fields++;
        }
    }
  rec_mset_iterator_free (&iter);

  return result;
}

void
rec_record_remove_field_by_name (rec_record_t record,
                                 const char *field_name,
                                 size_t n)
{
  rec_field_t field;
  rec_mset_iterator_t iter;
  rec_mset_elem_t elem;
  int num_fields = 0;

  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void**) &field, &elem))
    {
      if (rec_field_name_equal_p (rec_field_name (field), field_name))
        {
          if ((n == -1) || (n == num_fields))
            {
              rec_mset_remove_elem (record->mset, elem);
            }

          num_fields++;
        }
    }
  rec_mset_iterator_free (&iter);
}

size_t
rec_record_get_field_index_by_name (rec_record_t record,
                                    rec_field_t field)
{
  size_t res = 0;
  rec_mset_iterator_t iter;
  rec_field_t list_field;

  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &list_field, NULL))
    {
      if (field == list_field)
        {
          break;
        }

      if (rec_field_equal_p (field, list_field))
        {
          res++;
        }
    }

  rec_mset_iterator_free (&iter);

  return res;
}

rec_comment_t
rec_record_to_comment (rec_record_t record)
{
  rec_buf_t buf;
  rec_comment_t res;
  char *comment_str;
  size_t comment_str_size;
  rec_mset_iterator_t iter;
  rec_mset_elem_t elem;
  const void *data;

  buf = rec_buf_new (&comment_str, &comment_str_size);

  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_ANY, &data, &elem))
    {
      if (rec_mset_elem_type (elem) == MSET_FIELD)
        {
          rec_buf_puts (rec_write_field_str ((rec_field_t) data,
                                             REC_WRITER_NORMAL),
                        buf);
          rec_buf_putc ('\n', buf);
        }
      else
        {
          /* Comment.  */
          rec_buf_puts (rec_write_comment_str (rec_comment_text ((rec_comment_t) data),
                                               REC_WRITER_NORMAL),
                        buf);
          rec_buf_putc ('\n', buf);
        }
    }

  rec_mset_iterator_free (&iter);

  rec_buf_close (buf);

  /* Remove a trailing newline.  */
  if (comment_str[comment_str_size - 1] == '\n')
    {
      comment_str[comment_str_size - 1] = '\0';
    }

  res = rec_comment_new (comment_str);
  free (comment_str);

  return res;
}

char *
rec_record_source (rec_record_t record)
{
  char *res;

  if (record->source)
    {
      res = record->source;
    }
  else
    {
      res = "";
    }

  return res;
}

void
rec_record_set_source (rec_record_t record,
                       char *source)
{
  if (record->source)
    {
      free (record->source);
      record->source = NULL;
    }

  record->source = strdup (source);
}

size_t
rec_record_location (rec_record_t record)
{
  return record->location;
}

char *
rec_record_location_str (rec_record_t record)
{
  char *res;

  if (record->location_str)
    {
      res = record->location_str;
    }
  else
    {
      res = "";
    }

  return res;
}

void
rec_record_set_location (rec_record_t record,
                         size_t location)
{
  record->location = location;

  if (record->location_str)
    {
      free (record->location_str);
      record->location_str = NULL;
    }

  asprintf (&(record->location_str), "%zu", record->location);
}

size_t
rec_record_char_location (rec_record_t record)
{
  return record->char_location;
}

char *
rec_record_char_location_str (rec_record_t record)
{
  char *res;

  if (record->char_location_str)
    {
      res = record->char_location_str;
    }
  else
    {
      res = "";
    }

  return res;
}

void
rec_record_set_char_location (rec_record_t record,
                              size_t location)
{
  record->char_location = location;

  if (record->char_location_str)
    {
      free (record->char_location_str);
      record->char_location_str = NULL;
    }
  
  asprintf (&(record->char_location_str), "%zu", record->char_location);
}

bool
rec_record_contains_value (rec_record_t record,
                           const char *str,
                           bool case_insensitive)
{
  bool res = false;
  rec_mset_iterator_t iter;
  rec_field_t field;
  const char *field_value;
  char *occur;

  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
    {
      field_value = rec_field_value (field);

      if (case_insensitive)
        {
          occur = strcasestr (field_value, str);
        }
      else
        {
          occur = strstr (field_value, str);
        }

      res = (occur != NULL);
      if (res)
        {
          break;
        }
    }

  rec_mset_iterator_free (&iter);

  return res;
}

bool
rec_record_contains_field (rec_record_t record,
                           const char *field_name,
                           const char *field_value)
{
  bool res = false;
  rec_mset_iterator_t iter;
  rec_field_t field;

  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
    {
      if (rec_field_name_equal_p (field_name,
                                  rec_field_name (field))
          && (strcmp (field_value, rec_field_value (field)) == 0))
        {
          res = true;
          break;
        }
    }
  rec_mset_iterator_free (&iter);

  return res;
}

void *
rec_record_container (rec_record_t record)
{
  return record->container;
}

void
rec_record_set_container (rec_record_t record, void *container)
{
  record->container = container;
}

void
rec_record_uniq (rec_record_t record)
{
  rec_mset_iterator_t iter1, iter2;
  rec_mset_elem_t elem1;
  rec_field_t field1, field2;
  bool *to_remove;
  size_t num_fields, i, j;

  /* Create a map of the fields to remove and initialize all the
     entries to false.  */

  num_fields = rec_record_num_fields (record);
  to_remove = malloc (sizeof(bool) * num_fields);
  memset (to_remove, false, num_fields);

  /* Iterate on the fields of the record an mark the fields that will
     be removed in the removal mask.  */

  i = 0;
  iter1 = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter1, MSET_FIELD, (const void **) &field1, NULL))
    {
      if (!to_remove[i])
        {
          /* Mark any other occurrence of this field having the same
             value for removal. */

          j = 0;
          iter2 = rec_mset_iterator (record->mset);
          while (rec_mset_iterator_next (&iter2, MSET_FIELD, (const void **) &field2, NULL))
            {
              if ((j != i)
                  && rec_field_name_equal_p (rec_field_name (field1), rec_field_name (field2))
                  && rec_field_name_equal_p (rec_field_value (field1), rec_field_value (field2)))
                {
                  to_remove[j] = true;
                }
              
              j++;
            }
          rec_mset_iterator_free (&iter2);
        }

      i++;
    }
  rec_mset_iterator_free (&iter1);


  /* Remove the fields marked for removal.  */

  i = 0;
  iter1 = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter1, MSET_FIELD, (const void **) &field1, &elem1))
    {
      if (to_remove[i])
        {
          rec_mset_remove_elem (record->mset, elem1);
        }

      i++;
    }

  /* Cleanup.  */

  free (to_remove);
}

void
rec_record_append (rec_record_t dest_record,
                   rec_record_t src_record)
{
  rec_mset_iterator_t iter;
  rec_field_t field;

  iter = rec_mset_iterator (src_record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
    {
      if (!rec_mset_append (rec_record_mset (dest_record),
                            MSET_FIELD,
                            (void *) rec_field_dup (field),
                            MSET_FIELD))
        {
          /* Out of memory.  Just return.  */
          return;
        }
    }
  rec_mset_iterator_free (&iter);
}

void
rec_record_reset_marks (rec_record_t record)
{
  rec_mset_iterator_t iter;
  rec_field_t field;

  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
    {
      rec_field_set_mark (field, 0);
    }
  rec_mset_iterator_free (&iter);
}

bool
rec_record_mark_field (rec_record_t record,
                       rec_field_t field,
                       int mark)
{
  rec_mset_iterator_t iter;
  rec_field_t iter_field;

  iter = rec_mset_iterator (record->mset);
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &iter_field, NULL))
    {
      if (field == iter_field)
        {
          rec_field_set_mark (field, mark);
          rec_mset_iterator_free (&iter);
          return true;
        }
    }
  rec_mset_iterator_free (&iter);

  return false;
}

int
rec_record_field_mark (rec_record_t record,
                       rec_field_t field)
{
  return rec_field_mark (field);
}

/*
 * Private functions
 */

static void
rec_record_init (rec_record_t record)
{
  /* Initialize the record structure so it can be safely passed to
     rec_record_destroy even if its contents are not completely
     initialized with real values.  */
  
  memset (record, 0 /* NULL */, sizeof (struct rec_record_s));
}

static void
rec_record_field_disp_fn (void *data)
{
  rec_field_destroy ((rec_field_t) data);
}

static bool
rec_record_field_equal_fn (void *data1,
                           void *data2)
{
  return (data1 == data2);
  /*  return rec_field_equal_p ((rec_field_t) data1,
      (rec_field_t) data2);*/
}

static void *
rec_record_field_dup_fn (void *data)
{
  rec_field_t copy;

  copy = rec_field_dup ((rec_field_t) data);
  return (void *) copy;
}

static void
rec_record_comment_disp_fn (void *data)
{
  rec_comment_destroy ((rec_comment_t) data);
}

static bool
rec_record_comment_equal_fn (void *data1,
                             void *data2)
{
  return (data1 == data2);
/*  return rec_comment_equal_p ((rec_comment_t) data1,
(rec_comment_t) data2);*/
}

static void *
rec_record_comment_dup_fn (void *data)
{
  rec_comment_t copy;
  
  copy = rec_comment_dup ((rec_comment_t) data);
  return (void *) copy;
}

/* End of rec-record.c */
