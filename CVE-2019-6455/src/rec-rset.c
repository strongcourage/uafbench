/* -*- mode: C -*-
 *
 *       File:         rec-rset.c
 *       Date:         Thu Mar  5 18:12:10 2009
 *
 *       GNU recutils - Record Sets
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
#include <stdint.h>
#include <errno.h>
#include <locale.h>
#include <string.h>
#include <parse-datetime.h>

#if defined UUID_TYPE
#  include <uuid/uuid.h>
#endif

#include <rec.h>
#include <rec-utils.h>

/* Record Set Data Structures.
 *
 * A record set is a set of zero or more non-special records
 * intermixed with comments, maybe preceded by a record descriptor.
 */

/* The fprops structure contains useful properties associated with
   fields.  Those properties are usually extracted from the record
   descriptor of the rset.  */

struct rec_rset_fprops_s
{
  char *fname;

  bool key_p;          /* Primary key  */
  bool auto_p;         /* Auto-field.  */
#if defined REC_CRYPT_SUPPORT
  bool confidential_p; /* Confidential field.  */
#endif
  rec_type_t type; /* The field has an anonymous type.  */
  char *type_name; /* The field has a type in the types registry.  */

  struct rec_rset_fprops_s *next;
};

typedef struct rec_rset_fprops_s *rec_rset_fprops_t;

/* The rec_rset_s structure contains the data associated with a record
   set.  */

struct rec_rset_s
{
  rec_record_t descriptor;
  size_t descriptor_pos; /* Position of the descriptor into the record
                            set.  Some comments can appear before the
                            record descriptor in the rec file, so we
                            need to track it in order to write back
                            the record properly.  */

  /* Field properties.  */
  rec_rset_fprops_t field_props;

  /* Type registry.  */
  rec_type_reg_t type_reg;

  /* Simple fex containing the fields to use for ordering the records
     of this record descriptor.  */
  rec_fex_t order_by_fields;

  /* Size constraints.  */
  size_t min_size;
  size_t max_size;

  /* Sex-driven constraints.  */
  rec_sex_t *constraints;
  size_t num_constraints;

  /* Storage for records and comments.  */
  int record_type;
  int comment_type;
  rec_mset_t mset;
};

/* Static functions implemented below.  */

static void rec_rset_init (rec_rset_t rset);

static void rec_rset_update_types (rec_rset_t rset);
static void rec_rset_update_field_props (rec_rset_t rset);
static void rec_rset_update_size_constraints (rec_rset_t rset);
static void rec_rset_update_sex_constraints (rec_rset_t rset);

static bool rec_rset_record_equal_fn (void *data1, void *data2);
static void rec_rset_record_disp_fn (void *data);
static void *rec_rset_record_dup_fn (void *data);
static int  rec_rset_record_compare_fn (void *data1, void *data2, int type1);

static bool rec_rset_comment_equal_fn (void *data1, void *data2);
static void rec_rset_comment_disp_fn (void *data);
static void *rec_rset_comment_dup_fn (void *data);
static int  rec_rset_comment_compare_fn (void *data1, void *data2, int type2);

static bool rec_rset_type_field_p (const char *str);
static rec_fex_t rec_rset_type_field_fex (const char *str);
static char *rec_rset_type_field_type (const char *str);

static rec_rset_fprops_t rec_rset_get_props (rec_rset_t rset,
                                             const char *fname,
                                             bool create_p);

static bool rec_rset_add_auto_field_int (rec_rset_t rset,
                                         const char *field_name,
                                         rec_record_t record);
static bool rec_rset_add_auto_field_date (rec_rset_t rset,
                                          const char *field_name,
                                          rec_record_t record);

#if defined UUID_TYPE
static bool rec_rset_add_auto_field_uuid (rec_rset_t rset,
                                          const char *field_name,
                                          rec_record_t record);
#endif

static rec_record_t rec_rset_merge_records (rec_record_t to_record,
                                            rec_record_t from_record,
                                            rec_fex_t excluded_fields);

static int rec_rset_compare_typed_records (rec_rset_t rset,
                                           rec_record_t record1,
                                           rec_record_t record2,
                                           rec_fex_t fields);
                                           

/* The following macro is used by some functions to reduce
   verbosity.  */

#define FNAME(id) rec_std_field_name ((id))

/*
 * Public functions.
 */

rec_rset_t
rec_rset_new (void)
{
  rec_rset_t rset;
  
  rset = malloc (sizeof (struct rec_rset_s));
  if (rset)
    {
      rec_rset_init (rset);

      /* Create the mset.  */
      rset->mset = rec_mset_new ();
      if (rset->mset)
        {
          /* No descriptor, initially.  */
          rset->descriptor = NULL;
          rset->descriptor_pos = 0;
          rset->min_size = 0;
          rset->max_size = SIZE_MAX;
          rset->constraints = NULL;
          rset->num_constraints = 0;

          /* Create an empty type registry.  */
          rset->type_reg = rec_type_reg_new ();
          if (!rset->type_reg)
            {
              /* Out of memory.  */
              rec_rset_destroy (rset);
              return NULL;
            }

          /* No field properties, initially.  */
          rset->field_props = NULL;

          /* No order by field, initially.  */
          rset->order_by_fields = NULL;

          /* register the types.  See rec.h for the definition of
             MSET_COMMENT and MSET_RECORD.  */

          rset->record_type = rec_mset_register_type (rset->mset,
                                                      "record",
                                                      rec_rset_record_disp_fn,
                                                      rec_rset_record_equal_fn,
                                                      rec_rset_record_dup_fn,
                                                      rec_rset_record_compare_fn);
          rset->comment_type = rec_mset_register_type (rset->mset,
                                                       "comment",
                                                       rec_rset_comment_disp_fn,
                                                       rec_rset_comment_equal_fn,
                                                       rec_rset_comment_dup_fn,
                                                       rec_rset_comment_compare_fn);
        }
      else
        {
          /* Out of memory.  */

          rec_rset_destroy (rset);
          rset = NULL;
        }
    }

  return rset;
}

void
rec_rset_destroy (rec_rset_t rset)
{
  rec_rset_fprops_t props, aux = NULL;
  size_t i = 0;

  if (rset)
    {
      rec_record_destroy (rset->descriptor);
      rec_type_reg_destroy (rset->type_reg);

      for (i = 0; i < rset->num_constraints; i++)
        {
          rec_sex_destroy (rset->constraints[i]);
        }
      free (rset->constraints);

      props = rset->field_props;
      while (props)
        {
          aux = props;
          
          if (aux->type)
            {
              rec_type_destroy (aux->type);
            }
          free (aux->fname);
          free (aux->type_name);
          props = props->next;
          free (aux);
        }

      rec_fex_destroy (rset->order_by_fields);

      rec_mset_destroy (rset->mset);
      free (rset);
    }
}

rec_rset_t
rec_rset_dup (rec_rset_t rset)
{
  rec_rset_t new = NULL;

  new = malloc (sizeof (struct rec_rset_s));
  if (new)
    {
      rec_rset_init (new);

      new->record_type = rset->record_type;
      new->comment_type = rset->comment_type;
      new->mset = NULL;
      new->min_size = rset->min_size;
      new->max_size = rset->max_size;
      /* XXX: make copies of the following structures.  */
      new->type_reg = NULL;
      new->field_props = NULL;
      new->constraints = NULL;
      new->num_constraints = 0;

      if (rset->order_by_fields)
        {
          new->order_by_fields = rec_fex_dup (rset->order_by_fields);
          if (!new->order_by_fields)
            {
              /* Out of memory.  */
              rec_rset_destroy (new);
              return NULL;
            }
        }
    }

  new->mset = rec_mset_dup (rset->mset);
  if (!new->mset)
    {
      /* Out of memory.  */
      rec_rset_destroy (new);
      return NULL;
    }
  
  return new;
}

rec_mset_t
rec_rset_mset (rec_rset_t rset)
{
  return rset->mset;
}

size_t
rec_rset_num_elems (rec_rset_t rset)
{
  return rec_mset_count (rset->mset, MSET_ANY);
}

size_t
rec_rset_num_records (rec_rset_t rset)
{
  return rec_mset_count (rset->mset, rset->record_type);
}

size_t
rec_rset_num_comments (rec_rset_t rset)
{
  return rec_mset_count (rset->mset, rset->comment_type);
}

rec_record_t
rec_rset_descriptor (rec_rset_t rset)
{
  return rset->descriptor;
}

void
rec_rset_set_descriptor (rec_rset_t rset, rec_record_t record)
{
  if (rset->descriptor)
    {
      rec_record_destroy (rset->descriptor);
      rset->descriptor = NULL;
    }
  rset->descriptor = record;

  /* Update the types registry and the auto fields.  */
  rec_rset_update_types (rset);
  rec_rset_update_field_props (rset);
  rec_rset_update_size_constraints (rset);
  rec_rset_update_sex_constraints (rset);
}

size_t
rec_rset_descriptor_pos (rec_rset_t rset)
{
  return rset->descriptor_pos;
}

void
rec_rset_set_descriptor_pos (rec_rset_t rset,
                             size_t position)
{
  rset->descriptor_pos = position;
}

void
rec_rset_set_type (rec_rset_t rset,
                   const char *type)
{
  rec_field_t rec_field;

  if (!type)
    {
      /* This is a no-op for the default record set.  */
      return;
    }

  if (!rset->descriptor)
    {
      /* Create a record descriptor.  */
      rset->descriptor = rec_record_new ();
      
    }

  rec_field = rec_record_get_field_by_name (rset->descriptor,
                                            FNAME(REC_FIELD_REC),
                                            0);

  if (rec_field)
    {
      rec_field_set_value (rec_field, type);
    }
  else
    {
      rec_field = rec_field_new (FNAME(REC_FIELD_REC), type);
      rec_mset_append (rec_record_mset (rset->descriptor), MSET_FIELD, (void *) rec_field, MSET_FIELD);
    }
}

char *
rec_rset_type (rec_rset_t rset)
{
  char *res;
  rec_field_t field;

  res = NULL;
  if (rset->descriptor)
    {
      field = rec_record_get_field_by_name (rset->descriptor,
                                            FNAME(REC_FIELD_REC),
                                            0);
      if (field)
        {
          res = rec_extract_type (rec_field_value (field));
        }
    }

  return res;
}

char *
rec_rset_url (rec_rset_t rset)
{
  char *res;
  rec_field_t field;

  res = NULL;
  if (rset->descriptor)
    {
      field = rec_record_get_field_by_name (rset->descriptor,
                                            FNAME(REC_FIELD_REC),
                                            0);
      if (field)
        {
          res = rec_extract_url (rec_field_value (field));
        }
    }

  return res;
}

rec_type_reg_t
rec_rset_get_type_reg (rec_rset_t rset)
{
  return rset->type_reg;
}

void
rec_rset_rename_field (rec_rset_t rset,
                       const char *field_name,
                       const char *new_field_name)
{
  size_t j;
  rec_record_t descriptor;
  rec_fex_t fex;
  char *fex_str;
  char *type_str;
  rec_buf_t buf;
  char *result;
  size_t result_size;
  rec_fex_elem_t fex_elem;
  const char *fex_fname;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      rec_mset_t descriptor_mset = rec_record_mset (descriptor);
      rec_mset_iterator_t iter = rec_mset_iterator (descriptor_mset);
      rec_field_t field;

      while (rec_mset_iterator_next (&iter, MSET_FIELD, (void *) &field, NULL))
        {
          if (rec_field_name_equal_p (rec_field_name (field), FNAME(REC_FIELD_TYPE)))
            {
              /* Process a %type entry.  Invalid entries are
                 skipped.  */
              if (!rec_rset_type_field_p (rec_field_value (field)))
                {
                  continue;
                }

              fex = rec_rset_type_field_fex (rec_field_value (field));
              if (fex)
                {
                  for (j = 0; j < rec_fex_size (fex); j++)
                    {
                      fex_elem = rec_fex_get (fex, j);
                      fex_fname = rec_fex_elem_field_name (fex_elem);
                      if (rec_field_name_equal_p (field_name, fex_fname))
                        {
                          /* Replace it with new_field_name.  */
                          rec_fex_elem_set_field_name (fex_elem, new_field_name);
                        }
                    }

                  fex_str = rec_fex_str (fex, REC_FEX_CSV);
                  type_str = rec_rset_type_field_type (rec_field_value (field));
                  
                  buf = rec_buf_new (&result, &result_size);
                  rec_buf_puts (fex_str, buf);
                  rec_buf_putc (' ', buf);
                  rec_buf_puts (type_str, buf);
                  rec_buf_close (buf);

                  rec_field_set_value (field, result);

                  free (fex_str);
                  free (type_str);
                  rec_fex_destroy (fex);
                }
            }
          else if (rec_field_name_equal_p (rec_field_name (field), FNAME(REC_FIELD_KEY))
                   || rec_field_name_equal_p (rec_field_name (field), FNAME(REC_FIELD_MANDATORY))
                   || rec_field_name_equal_p (rec_field_name (field), FNAME(REC_FIELD_UNIQUE))
                   || rec_field_name_equal_p (rec_field_name (field), FNAME(REC_FIELD_PROHIBIT))
#if defined REC_CRYPT_SUPPORT
                   || rec_field_name_equal_p (rec_field_name (field), FNAME(REC_FIELD_CONFIDENTIAL))
#endif
                   || rec_field_name_equal_p (rec_field_name (field), FNAME(REC_FIELD_SORT)))
            {
              /* Rename the field in the fex expression that is the
                 value of the field.  Skip invalid entries.  */
              fex = rec_fex_new (rec_field_value (field), REC_FEX_SIMPLE);
              if (fex)
                {
                  for (j = 0; j < rec_fex_size (fex); j++)
                    {
                      fex_elem = rec_fex_get (fex, j);

                      fex_fname = rec_fex_elem_field_name (fex_elem);
                      if (rec_field_name_equal_p (field_name, fex_fname))
                        {
                          /* Replace it with new_field_name.  */
                          rec_fex_elem_set_field_name (fex_elem, new_field_name);
                        }
                    }
                  
                  fex_str = rec_fex_str (fex, REC_FEX_SIMPLE);
                  rec_field_set_value (field, fex_str);
                  free (fex_str);
                }
            }          
        }

      rec_mset_iterator_free (&iter);
    }

  /* Update the types registry.  */
  rec_rset_update_field_props (rset);
}

const char *
rec_rset_key (rec_rset_t rset)
{
  const char *key = NULL;
  rec_rset_fprops_t props = rset->field_props;

  while (props)
    {
      if (props->key_p)
        {
          /* There must be only one field marked as key.  */
          key = props->fname;
          break;
        }
      props = props->next;
    }
  
  return key;
}

rec_fex_t
rec_rset_auto (rec_rset_t rset)
{
  rec_fex_t fex;
  rec_rset_fprops_t props;

  fex = rec_fex_new (NULL, REC_FEX_SIMPLE);

  props = rset->field_props;
  while (props)
    {
      if (props->auto_p)
        {
          rec_fex_append (fex,
                          props->fname,
                          -1, -1);
        }
      props = props->next;
    }

  return fex;
}

#if defined REC_CRYPT_SUPPORT

bool
rec_rset_field_confidential_p (rec_rset_t rset,
                               const char *field_name)
{
  rec_fex_t fex;
  size_t fex_size;
  size_t i;
  bool result = false;
  const char *fex_field_name;

  fex = rec_rset_confidential (rset);
  fex_size = rec_fex_size (fex);

  for (i = 0; i < fex_size; i++)
    {
      fex_field_name = rec_fex_elem_field_name (rec_fex_get (fex, i));
      if (rec_field_name_equal_p (field_name, fex_field_name))
        {
          result = true;
          break;
        }
    }

  return result;
}

rec_fex_t
rec_rset_confidential (rec_rset_t rset)
{
  rec_fex_t fex;
  rec_rset_fprops_t props;

  fex = rec_fex_new (NULL, REC_FEX_SIMPLE);

  props = rset->field_props;
  while (props)
    {
      if (props->confidential_p)
        {
          rec_fex_append (fex,
                          props->fname,
                          -1, -1);
        }

      props = props->next;
    }

  return fex;
}

#endif /* REC_CRYPT_SUPPORT */

rec_type_t
rec_rset_get_field_type (rec_rset_t rset,
                         const char *field_name)
{
  rec_type_t type = NULL;
  rec_rset_fprops_t props = NULL;

  props = rec_rset_get_props (rset, field_name, false);
  if (props)
    {
      type = props->type;
      if (!type && props->type_name)
        {
          type = rec_type_reg_get (rset->type_reg, props->type_name);
        }
    }
  
  return type;
}

size_t
rec_rset_min_records (rec_rset_t rset)
{
  return rset->min_size;
}

size_t
rec_rset_max_records (rec_rset_t rset)
{
  return rset->max_size;
}

char *
rec_rset_source (rec_rset_t rset)
{
  rec_record_t record;

  /* The source of the record set is considered to be the source of
     its first record: either the descriptor or some other record.  */
  
  record = rec_rset_descriptor (rset);
  if (!record)
    {
      record = (rec_record_t) rec_mset_get_at (rset->mset, MSET_RECORD, 0);
    }

  return rec_record_source (record);
}


bool
rec_rset_set_order_by_fields (rec_rset_t rset,
                              rec_fex_t field_names)
{
  rec_fex_destroy (rset->order_by_fields);
  rset->order_by_fields = rec_fex_dup (field_names);
  return (rset->order_by_fields != NULL);
}

rec_fex_t
rec_rset_order_by_fields (rec_rset_t rset)
{
  return rset->order_by_fields;
}

rec_rset_t
rec_rset_sort (rec_rset_t rset,
               rec_fex_t sort_by)
{
  if (sort_by)
    {
      rec_rset_set_order_by_fields (rset, sort_by);
    }

  if (rset->order_by_fields)
    {
      /* Duplicate the multi-set indicating that the elements must be
         sorted.  */

      if (!rec_mset_sort (rset->mset))
        {
          /* Out of memory.  */
          return NULL;
        }

      /* Update field properties, in case order_by_fields was changed
         above.  */
  
      rec_rset_update_field_props (rset);
    }

  return rset;
}

rec_rset_t
rec_rset_group (rec_rset_t rset,
                rec_fex_t group_by)
{
  rec_mset_iterator_t iter;
  rec_record_t record;
  rec_mset_elem_t elem;
  size_t map_size;
  bool *deletion_map;
  size_t num_record;

  /* Create and initialize the deletion map.  */

  map_size = sizeof(bool) * rec_rset_num_records (rset);
  deletion_map = malloc (map_size);
  if (!deletion_map)
    {
      /* Out of memory.  */
      return NULL;
    }

  memset (deletion_map, false, map_size);

  /* Iterate on the records of RSET, grouping records and marking the
     grouped records for deletion.  */

  num_record = 0;
  iter = rec_mset_iterator (rec_rset_mset (rset));
  while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **)&record, NULL))
    {
      if (!deletion_map[num_record])
        {
          size_t num_record_2 = num_record;
          rec_mset_iterator_t iter2 = iter;
          rec_record_t record2;
          
          while (rec_mset_iterator_next (&iter2, MSET_RECORD, (const void**)&record2, NULL))
            {
              num_record_2++;
              
              if (rec_rset_compare_typed_records (rset, record, record2, group_by) != 0)
                {
                  break;
                }
              else
                {
                  /* Insert all the elements of record2 into record,
                     but not the group-by fields.  Also, remove any
                     duplicated field created in record2 as the result
                     of the operation.  */

                  if (!rec_rset_merge_records (record,
                                               record2,
                                               group_by))
                    {
                      /* Out of memory.  */
                      return NULL;
                    }
                  
                  /* Mark record2 for removal.  */
                  deletion_map[num_record_2] = true;
                }
            }
        }

      num_record++;
    }
  rec_mset_iterator_free (&iter);

  /* Delete the records marked for deletion.  */

  num_record = 0;
  iter = rec_mset_iterator (rec_rset_mset (rset));
  while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &record, &elem))
    {
      if (deletion_map[num_record])
        {
          rec_mset_remove_elem (rec_rset_mset (rset), elem);
        }

      num_record++;
    }
  rec_mset_iterator_free (&iter);

  free (deletion_map);

  return rset;
}

rec_rset_t
rec_rset_add_auto_fields (rec_rset_t rset,
                          rec_record_t record)
{
  rec_fex_t auto_fields;
  rec_type_t type;
  size_t i;

  if ((auto_fields = rec_rset_auto (rset)))
    {
      size_t num_auto_fields = rec_fex_size (auto_fields);

      for (i = 0; i < num_auto_fields; i++)
        {
          const char *auto_field_name =
            rec_fex_elem_field_name (rec_fex_get (auto_fields, i));

          if (!rec_record_field_p (record, auto_field_name))
            {
              /* The auto field is not already present in record, so
                 add one automatically.  Depending on its type the
                 value is calculated differently.  If the record does
                 not have a type, or the type is incorrect, ignore
                 it.  */
              
              type = rec_rset_get_field_type (rset, auto_field_name);
              if (type)
                {
                  switch (rec_type_kind (type))
                    {
                    case REC_TYPE_INT:
                    case REC_TYPE_RANGE:
                      {
                        if (!rec_rset_add_auto_field_int (rset, auto_field_name, record))
                          {
                            /* Out of memory.  */
                            return NULL;
                          }

                        break;
                      }
                    case REC_TYPE_DATE:
                      {
                        if (!rec_rset_add_auto_field_date (rset, auto_field_name, record))
                          {
                            /* Out of memory.  */
                            return NULL;
                          }

                        break;
                      }
#if defined UUID_TYPE
                    case REC_TYPE_UUID:
                      {
                        if (!rec_rset_add_auto_field_uuid (rset, auto_field_name, record))
                          {
                            /* Out of memory.  */
                            return NULL;
                          }
                        
                        break;
                      }
#endif /* UUID_TYPE */
                    default:
                      {
                        /* Do nothing for other types.  */
                        break;
                      }
                    }
                }
            }
        }
    }

  return rset;
}

size_t
rec_rset_num_sex_constraints (rec_rset_t rset)
{
  return rset->num_constraints;
}

rec_sex_t
rec_rset_sex_constraint (rec_rset_t rset,
                         size_t index)
{
  return rset->constraints[index];
}

/*
 * Private functions
 */

static void
rec_rset_init (rec_rset_t rset)
{
  /* Initialize the rset structure so it can be safely passed to
     rec_rset_destroy even if its contents are not completely
     initialized with real values.  */

  memset (rset, 0 /* NULL */, sizeof (struct rec_rset_s));
}

static void
rec_rset_record_disp_fn (void *data)
{
  rec_record_t record = (rec_record_t) data;
  rec_record_destroy (record);
}

static bool
rec_rset_record_equal_fn (void *data1,
                          void *data2)
{
  return (data1 == data2);
  /*  return rec_record_equal_p ((rec_record_t) data1,
      (rec_record_t) data2); */
}

static void *
rec_rset_record_dup_fn (void *data)
{
  rec_record_t record = (rec_record_t) data;
  rec_record_t new = rec_record_dup (record);

  return (void *) new;
}

static int
rec_rset_record_compare_fn (void *data1,
                            void *data2,
                            int type2)
{
  /* data1 is a record.  data2 can be either a record or a comment.

     order_by_field can't be NULL, because this callback is invoked
     only if rec_mset_add_sorted is used to add an element to the
     list.

     The following rules apply here:
     
     1. If the fields in order_by_fields are not in both record1 and
        record2, then data1 < data2.
 
     2. Else, perform a lexicographic comparison, i.e.

        (a1, a2, ...) < (b1, b2, ...) IFF
                      a1 < b1 OR (a1 = b2 AND a2 < b2) OR ...

    Note that record1 will always be a regular record.  Never a
    descriptor.
  */

  rec_rset_t rset                  = NULL;
  rec_record_t record1             = NULL;
  rec_record_t record2             = NULL;
  int type_comparison              = 0;

  /* If elem2 is a comment then elem1 > elem2.  */
  if (type2 == MSET_COMMENT)
    {
      return 1;
    }

  /* Get the records and the containing rset.  */
  record1 = (rec_record_t) data1;
  record2 = (rec_record_t) data2;
  rset = (rec_rset_t) rec_record_container (record1);

  /* Perform a lexicographic comparison of the order_by_fields in both
     registers.  */

  type_comparison = rec_rset_compare_typed_records (rset,
                                                    record1,
                                                    record2,
                                                    rset->order_by_fields);

  /* If both records are equal, return -1 instead of 0 in order to
     maintain the relative ordering between equal records.  */

  if (type_comparison == 0)
    {
      type_comparison = -1;
    }
  
  return type_comparison;
}

static void
rec_rset_comment_disp_fn (void *data)
{
  rec_comment_t comment = (rec_comment_t) data;
  rec_comment_destroy (comment);
}

static bool
rec_rset_comment_equal_fn (void *data1,
                           void *data2)
{
  return (data1 == data2);
  /*  return rec_comment_equal_p ((rec_comment_t) data1,
      (rec_comment_t) data2);*/
}

static void *
rec_rset_comment_dup_fn (void *data)
{
  rec_comment_t comment = (rec_comment_t) data;
  rec_comment_t new = rec_comment_dup (comment);
  return (void *) new;
}

static int
rec_rset_comment_compare_fn (void *data1,
                             void *data2,
                             int   type2)
{
  /* data1 is a comment, and data2 can be either a comment or a
     record.  In any case, data1 < data2.  */

  return -1;
}

static void
rec_rset_update_sex_constraints (rec_rset_t rset)
{
  /* Reset the existing constraints.  */

  {
    size_t i = 0;

    for (i = 0; i < rset->num_constraints; i++)
      {
        rec_sex_destroy (rset->constraints[i]);
      }
    rset->num_constraints = 0;
  }

  /* If there is not a record descriptor in the record set then simply
     return.  */

  if (!rset->descriptor)
    {
      return;
    }

  /* Allocate memory for the constraints memory.  In case of
     not-enough-memory simply return.  */

  {
    size_t num_constraints =
      rec_record_get_num_fields_by_name (rset->descriptor, FNAME(REC_FIELD_CONSTRAINT));
    rset->constraints = malloc (num_constraints * sizeof(rec_sex_t));

    if (!rset->constraints)
      {
        return;
      }
  }
  
  /* Scan the record descriptor for %constraint: directives, and build
     the constraints.  Not well formed constraint entries,
     i.e. entries not containing valid sexes, are simply ignored.  */

  {
    rec_field_t field = NULL;
    rec_mset_iterator_t iter;

    iter = rec_mset_iterator (rec_record_mset (rset->descriptor));
    while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **)&field, NULL))
      {
        const char *field_name = rec_field_name (field);
        const char *field_value = rec_field_value (field);

        if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_CONSTRAINT)))
          {
            rec_sex_t sex = rec_sex_new (false);
            if (!sex)
              {
                return;
              }

            if (rec_sex_compile (sex, field_value))
              {
                rset->constraints[rset->num_constraints++] = sex;
              }
            else
              {
                rec_sex_destroy (sex);
              }
          }
      }
    rec_mset_iterator_free (&iter);
  }
}

static void
rec_rset_update_size_constraints (rec_rset_t rset)
{
  rec_field_t field;
  enum rec_size_condition_e condition;
  size_t size = 0;

  /* Reset the constraints. */
  rset->min_size = 0;
  rset->max_size = SIZE_MAX;

  /* Scan the record descriptor for %size: directives, and build the
     new list.  */
  if (rset->descriptor)
    {
      field = rec_record_get_field_by_name (rset->descriptor,
                                            FNAME(REC_FIELD_SIZE),
                                            0);

      if (field && rec_match (rec_field_value (field), REC_INT_SIZE_RE))
        {
          /* Extract 'min' and 'max' and update the constraints in the
             rset.  */
          condition = rec_extract_size_condition (rec_field_value (field));
          size = rec_extract_size (rec_field_value (field));
          
          /* Set min_size and max_size depending on the
             condition.  */
          switch (condition)
            {
            case SIZE_COND_E:
              {
                rset->min_size = size;
                rset->max_size = size;
                break;
              }
            case SIZE_COND_L:
              {
                rset->max_size = size - 1;
                break;
              }
            case SIZE_COND_LE:
              {
                rset->max_size = size;
                break;
              }
            case SIZE_COND_G:
              {
                rset->min_size = size + 1;
                break;
              }
            case SIZE_COND_GE:
              {
                rset->min_size = size;
                break;
              }
            }
        }
    }
}

static void
rec_rset_update_field_props (rec_rset_t rset)
{
  rec_rset_fprops_t props = NULL;
#if defined REC_CRYPT_SUPPORT
  const char *confidential_field_name;
#endif
  char *type_name = NULL;

  /* Reset the field properties.  */
  props = rset->field_props;
  while (props)
    {
      props->key_p = false;
      props->auto_p = false;
      if (props->type)
        {
          rec_type_destroy (props->type);
          props->type = NULL;
        }

      props = props->next;
    }

  if (rset->descriptor)
    {
      /* Pass 1: scan the record descriptor for % directives, and update
         the fields properties accordingly.  */

      rec_field_t field;
      rec_mset_iterator_t iter;

      iter = rec_mset_iterator (rec_record_mset (rset->descriptor));
      while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void**) &field, NULL))
        {
          const char *field_name = rec_field_name (field);
          const char *field_value = rec_field_value (field);

          /* Update field types.  Only valid %type: descriptors are
             considered.  Invalid descriptors are ignored.  */

          if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_TYPE))
              && rec_rset_type_field_p (field_value))
            {
              size_t i;
              rec_fex_t fex = rec_rset_type_field_fex (field_value);

              for (i = 0; i < rec_fex_size (fex); i++)
                {
                  char *field_type = rec_rset_type_field_type (field_value);
                  rec_type_t type = rec_type_new (field_type);

                  if (!type)
                    {
                      /* Set field_type as a field property.  Note
                         that if the field is already associated with
                         an anonymous type, or a type name, they are
                         replaced.  */
                      
                      const char *p = field_type;
                      rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE, &type_name);
                      props = rec_rset_get_props (rset,
                                                  rec_fex_elem_field_name (rec_fex_get (fex, i)),
                                                  true);
                      if (props->type)
                        {
                          rec_type_destroy (props->type);
                          props->type = NULL;
                        }
                      free (props->type_name);
                      props->type_name = type_name;
                    }
                  else
                    {
                      /* Set the type as a field property.  Note that
                         if the field is already associated with an
                         anonymous type, or a type name, they are
                         replaced.  */

                      props = rec_rset_get_props (rset,
                                                  rec_fex_elem_field_name (rec_fex_get (fex, i)),
                                                  true);
                      if (props->type)
                        {
                          rec_type_destroy (props->type);
                        }
                      free (props->type_name);
                      props->type_name = NULL;
                      props->type = type;
                    }
                  
                  free (field_type);
                }

              rec_fex_destroy (fex);
            }

          /* Update the key field.  */
          if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_KEY)))
            {
              /* %key: fields containing incorrect data are
                  ignored.  */

              const char *field_value = rec_field_value (field);
              char *type_name = NULL;

              rec_skip_blanks (&field_value);
              rec_parse_regexp (&field_value, "^" REC_RECORD_TYPE_RE, &type_name);
              props = rec_rset_get_props (rset, type_name, true);
              props->key_p = true;
              free (type_name);
            }

          /* Update auto fields.  */
          if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_AUTO)))
            {
              /* %auto: fields containing incorrect data are
                  ignored.  */

              rec_fex_t fex = rec_fex_new (rec_field_value (field), REC_FEX_SIMPLE);
              if (fex)
                {
                  size_t i;

                  for (i = 0; i < rec_fex_size (fex); i++)
                    {
                      const char *auto_field_name
                        = rec_fex_elem_field_name (rec_fex_get (fex, i));
                      props = rec_rset_get_props (rset, auto_field_name, true);
                      props->auto_p = true;
                    }
                }
            }

          /* Update sort fields.  The last field takes precedence.  */
          if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_SORT)))
            {
              /* Parse the simple fex in the field value.  Invalid
                 entries are just ignored.  */
              
              const char *field_value = rec_field_value (field);

              rec_fex_t fex = rec_fex_new (field_value, REC_FEX_SIMPLE);
              if (fex)
                {
                  rec_fex_destroy (rset->order_by_fields);
                  rset->order_by_fields = fex;
                }
            }

#if defined REC_CRYPT_SUPPORT
          /* Update confidential fields.  */
          if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_CONFIDENTIAL)))
            {
              /* Parse the field names in the field value.  Ignore
                 invalid entries.  */

              rec_fex_t fex = rec_fex_new (rec_field_value (field), REC_FEX_SIMPLE);
              if (fex)
                {
                  size_t i;

                  for (i = 0; i < rec_fex_size (fex); i++)
                    {
                      confidential_field_name =
                        rec_fex_elem_field_name (rec_fex_get (fex, i));
                      props = rec_rset_get_props (rset, confidential_field_name, true);
                      props->confidential_p = true;
                    }
                }
            }
#endif /* REC_CRYPT_SUPPORT */

        }

      rec_mset_iterator_free (&iter);
    }

  /* Pass 2: scan the fields having properties on the record set.  */

  props = rset->field_props;
  while (props)
    {
      /* Auto fields not having an explicit type are implicitly
         typed as integers.  */
      
      if (props->auto_p && !props->type && !props->type_name)
        {
          props->type = rec_type_new ("int");
        }
      
      props = props->next;
    }
}

static void
rec_rset_update_types (rec_rset_t rset)
{
  rec_field_t field;
  rec_mset_iterator_t iter;
  const char *p, *q = NULL;
  rec_type_t type;
  char *type_name, *to_type = NULL;
  

  /* Scan the record descriptor for %typedef directives and update the
     types registry accordingly.  */
  if (rset->descriptor)
    {
      /* Purge the registry.  */

      rec_type_reg_destroy (rset->type_reg);
      rset->type_reg = rec_type_reg_new ();

      /* Iterate on the fields of the descriptor.  */

      iter = rec_mset_iterator (rec_record_mset (rset->descriptor));
      while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
        {
          const char *field_name = rec_field_name (field);
          const char *field_value = rec_field_value (field);

          if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_TYPEDEF)))
            {
              p = field_value;
              rec_skip_blanks (&p);

              /* Get the name of the type.  */
              if (rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE, &type_name))
                {
                  /* Get the type.  */
                  type = rec_type_new (p);
                  if (type)
                    {
                      /* Set the name of the type.  */
                      rec_type_set_name (type, type_name);

                      /* Create and insert the type in the type
                         registry.  */
                      rec_type_reg_add (rset->type_reg, type);
                    }
                  else
                    {
                      /* This could be a synonym.  Try to parse a type
                         name and, if the operation succeeds, insert
                         the synonym in the registry.  */
                      rec_skip_blanks (&p);
                      q = p;
                      if (rec_parse_regexp (&q,
                                            "^" REC_TYPE_NAME_RE "[ \t\n]*",
                                            NULL))
                        {
                          rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE, &to_type);
                          rec_type_reg_add_synonym (rset->type_reg,
                                                    type_name,
                                                    to_type);
                        }
                    }
                  
                  free (type_name);
                }
            }
        }

      rec_mset_iterator_free (&iter);
    }
}

static bool
rec_rset_type_field_p (const char *str)
{
  const char *p = str;

  /* Check the fex */

  rec_skip_blanks (&p);
  if (!rec_parse_regexp (&p,
                         "^" REC_FNAME_LIST_CS_RE,
                         NULL))
    {
      return false;
    }
  rec_skip_blanks (&p);

  /* Check the type description, or the name of a type.  */

  return (rec_type_descr_p (p)
          || rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE "[ \t\n]*$", NULL));
}

static rec_fex_t
rec_rset_type_field_fex (const char *str)
{
  rec_fex_t fex = NULL;
  const char *p;
  char *name;

  p = str;

  if (rec_parse_regexp (&p,
                        "^" REC_FNAME_LIST_CS_RE,
                        &name))
    {
      fex = rec_fex_new (name, REC_FEX_CSV);
      free (name);
    }

  return fex;
}

static char*
rec_rset_type_field_type (const char *str)
{
  char *result = NULL;
  const char *p;

  if (rec_rset_type_field_p (str))
    {
      p = str;

      rec_skip_blanks (&p);
      rec_parse_regexp (&p, "^" REC_FNAME_LIST_CS_RE, NULL);
      rec_skip_blanks (&p);

      /* Return the rest of the string.  */
      result = strdup (p);
    }

  return result;
}

static rec_rset_fprops_t
rec_rset_get_props (rec_rset_t rset,
                    const char *fname,
                    bool create_p)
{
  rec_rset_fprops_t props = NULL;
  
  props = rset->field_props;
  while (props)
    {
      if (rec_field_name_equal_p (fname, props->fname))
        {
          break;
        }

      props = props->next;
    }

  if (!props && create_p)
    {
      /* Create a new properties structure for this field name and
         initialize it.  */
      props = malloc (sizeof (struct rec_rset_fprops_s));
      if (props)
        {
          props->fname = strdup (fname);
          props->auto_p = false;
          props->key_p = false;

#if defined REC_CRYPT_SUPPORT
          props->confidential_p = false;
#endif

          props->type = NULL;
          props->type_name = NULL;
          
          /* Prepend it to the field properties list.  */
          props->next = rset->field_props;
          rset->field_props = props;
        }
    }

  return props;
}

static bool
rec_rset_add_auto_field_int (rec_rset_t rset,
                             const char *field_name,
                             rec_record_t record)
{
  rec_mset_iterator_t iter;
  rec_record_t rec;
  rec_field_t field;
  size_t num_fields, i;
  int auto_value, field_value;
  char *end;
  char *auto_value_str;

  /* Find the auto value.  */

  auto_value = 0;

  iter = rec_mset_iterator (rec_rset_mset (rset));
  while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &rec, NULL))
    {
      num_fields = rec_record_get_num_fields_by_name (rec, field_name);
      for (i = 0; i < num_fields; i++)
        {
          field = rec_record_get_field_by_name (rec, field_name, i);
          
          /* Ignore fields that can't be converted to integer
             values.  */
          errno = 0;
          field_value = strtol (rec_field_value (field), &end, 10);
          if ((errno == 0) && (*end == '\0'))
            {
              if (auto_value <= field_value)
                {
                  auto_value = field_value + 1;
                }
            }
        }
    }

  rec_mset_iterator_free (&iter);
       
  /* Create and insert the auto field.  */

  if (asprintf (&auto_value_str, "%d", auto_value) != -1)
    {
      field = rec_field_new (field_name, auto_value_str);
      if (!field)
        {
          /* Out of memory.  */
          free (auto_value_str);
          return false;
        }

      if (!rec_mset_insert_at (rec_record_mset (record), MSET_FIELD, (void *) field, 0))
        {
          /* Out of memory.  */
          free (auto_value_str);
          return false;
        }

      free (auto_value_str);
    }

  return true;
}

static bool
rec_rset_add_auto_field_date (rec_rset_t rset,
                              const char *field_name,
                              rec_record_t record)
{
  rec_field_t auto_field;
  time_t t;
  char outstr[200];
  struct tm *tmp;

  t = time (NULL);
  tmp = localtime (&t);

  setlocale (LC_TIME, "C"); /* We want english dates that can be
                                 parsed with parse_datetime */
  strftime (outstr, sizeof(outstr), "%a, %d %b %Y %T %z", tmp);
  setlocale (LC_TIME, ""); /* And restore the locale from the
                              environment. */

  auto_field = rec_field_new (field_name, outstr);
  if (!auto_field)
    {
      /* Out of memory.  */
      return false;
    }

  if (!rec_mset_insert_at (rec_record_mset (record), MSET_FIELD, (void *) auto_field, 0))
    {
      /* Out of memory.  */
      return false;
    }

  return true;
}

#if defined UUID_TYPE

static bool
rec_rset_add_auto_field_uuid (rec_rset_t rset,
                              const char *field_name,
                              rec_record_t record)
{
  rec_field_t auto_field;
  uuid_t uu;
  char uu_str[40]; /* Enough to hold any standard UUID.  */

  /* Generate a new time-based UUID using the libuuid library and use
     it for the value of the new auto field.  */

  uuid_generate_time (uu);
  uuid_unparse (uu, uu_str);
  
  auto_field = rec_field_new (field_name, uu_str);
  if (!auto_field)
    {
      /* Out of memory.  */
      return false;
    }

  if (!rec_mset_insert_at (rec_record_mset (record), MSET_FIELD, (void *) auto_field, 0))
    {
      /* Out of memory.  */
      return false;
    }

  return true;
}

#endif /* UUID_TYPE */

static rec_record_t
rec_rset_merge_records (rec_record_t to_record,
                        rec_record_t from_record,
                        rec_fex_t    group_by_fields)
{
  rec_mset_elem_t elem;
  void *data;
  rec_mset_iterator_t iter;

  iter = rec_mset_iterator (rec_record_mset (from_record));
  while (rec_mset_iterator_next (&iter, MSET_ANY, (const void**) &data, &elem))
    {
      if (rec_mset_elem_type (elem) == MSET_FIELD)
        {
          rec_field_t field = (rec_field_t) data;

          /* Don't add the field if it is in the list of group-by
             fields.  */

          if (rec_fex_member_p (group_by_fields, rec_field_name (field), -1, -1))
            {
              continue;
            }

          /* Don't allow duplicated fields in the resulting record
             generated as a result of this operation.  This is
             commented out because it fucks up the usage of aggregated
             functions in grouped-by record sets.  */

          /*         if (rec_record_contains_field (to_record,
                                         rec_field_name (field),
                                         rec_field_value (field)))
            {
              continue;
              } */

          /* Ok, add this field.  */

          if (!rec_mset_append (rec_record_mset (to_record),
                                MSET_FIELD,
                                (void *) rec_field_dup (field),
                                MSET_ANY))
            {
              /* Out of memory.  */
              return NULL;
            }
        }
      else
        {
          rec_comment_t comment = (rec_comment_t) data;
          rec_mset_append (rec_record_mset (to_record),
                           MSET_COMMENT,
                           (void *) rec_comment_dup (comment),
                           MSET_ANY);
        }
    }
  rec_mset_iterator_free (&iter);

  return to_record;
}

static int
rec_rset_compare_typed_records (rec_rset_t rset,
                                rec_record_t record1,
                                rec_record_t record2,
                                rec_fex_t fields)
{
  int result = 0;
  size_t i = 0;
  size_t num_fields = rec_fex_size (fields);

  for (i = 0; i < num_fields; i++)
    {
      rec_fex_elem_t elem       = rec_fex_get (fields, i);
      const char    *field_name = rec_fex_elem_field_name (elem);
      rec_field_t    field1     = rec_record_get_field_by_name (record1, field_name, 0);
      rec_field_t    field2     = rec_record_get_field_by_name (record2, field_name, 0);

      /* If any of the fields is not present in some of the records
         then that record is considered to be smaller than the record
         featuring the other one.  */

      if (field1 && !field2)
        {
          result = 1; /* field1 > field2 */
          break;
        }
      else if (!field1 && field2)
        {
          result = -1;  /* field1 < field2 */
          break;
        }
      else if (!field1 && !field2)
        {
          result = -1;  /* field1 < field2 */
          break;
        }

      /* A field with such a name exists in both records.  Compare the
         field typed values.  */
      
      result =  rec_type_values_cmp (rec_rset_get_field_type (rset, field_name),
                                     rec_field_value (field1),
                                     rec_field_value (field2));

      if (result != 0)
        {
          /* Either (a1, a2, ...) < (b1, b2, ...) or (a1, a2, ...) >
             (b1, b2, ...) */
          
          break;
        }
    }

  return result;
}

/* End of rec-rset.c */
