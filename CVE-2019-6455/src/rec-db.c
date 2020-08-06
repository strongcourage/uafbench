/* -*- mode: C -*-
 *
 *       File:         rec-db.c
 *       Date:         Thu Jan 14 15:35:27 2010
 *
 *       GNU recutils - Databases
 *
 */

/* Copyright (C) 2010-2019 Jose E. Marchesi */

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

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <gl_array_list.h>
#include <gl_list.h>

#include <rec-utils.h>
#include <rec.h>

/*
 * Data structures.
 */

struct rec_db_s
{
  size_t size;                    /* Number of record sets contained
                                     in this database.  */
  gl_list_t rset_list;            /* List of record sets.  */
  rec_aggregate_reg_t aggregates; /* Registry with the aggregates.  */
};

/* Static functions defined in this file.  */

static bool rec_db_rset_equals_fn (const void *elt1,
                                   const void *elt2);
static void rec_db_rset_dispose_fn (const void *elt);

static rec_record_t rec_db_process_fex (rec_db_t db,
                                        rec_rset_t rset,
                                        rec_record_t record,
                                        rec_fex_t fex);

static bool rec_db_record_selected_p (size_t num_rec,
                                      rec_record_t record,
                                      size_t *index,
                                      rec_sex_t sex,
                                      const char *fast_string,
                                      bool case_insensitive_p);
static void rec_db_add_random_indexes (size_t **index, size_t num, size_t limit);
static bool rec_db_index_p (size_t *index, size_t num);

static bool rec_db_set_act_rename (rec_rset_t rset, rec_record_t record, rec_fex_t fex, bool rename_descriptor, const char *arg);
static bool rec_db_set_act_set (rec_rset_t rset, rec_record_t record, rec_fex_t fex, bool xxx, const char *arg);
static bool rec_db_set_act_add (rec_rset_t rset, rec_record_t record, rec_fex_t fex, const char *arg);
static bool rec_db_set_act_delete (rec_rset_t rset, rec_record_t record, rec_fex_t fex, bool comment_out);

static rec_rset_t rec_db_join (rec_db_t db, const char *type1, const char *field, const char *type2);
static rec_record_t rec_db_merge_records (rec_record_t record1, rec_record_t record2, const char *prefix);

/*
 * Public functions.
 */

rec_db_t
rec_db_new (void)
{
  rec_db_t new;

  new = malloc (sizeof (struct rec_db_s));
  if (new)
    {
      new->size = 0;
      new->rset_list = gl_list_nx_create_empty (GL_ARRAY_LIST,
                                                rec_db_rset_equals_fn,
                                                NULL,
                                                rec_db_rset_dispose_fn,
                                                true);
      
      if (new->rset_list == NULL)
        {
          /* Out of memory.  */
          free (new);
          new = NULL;
        }

      /* Add the standard field functions to the registry in the
         database.  */

      new->aggregates = rec_aggregate_reg_new ();
      if (!new->aggregates)
        {
          /* Out of memory.  */
          free (new);
          return NULL;
        }
      rec_aggregate_reg_add_standard (new->aggregates);
    }

  return new;
}

void
rec_db_destroy (rec_db_t db)
{
  if (db)
    {
      rec_aggregate_reg_destroy (db->aggregates);
      gl_list_free (db->rset_list);
      free (db);
    }
}

size_t
rec_db_size (rec_db_t db)
{
  return db->size;
}

rec_rset_t
rec_db_get_rset (rec_db_t db,
                 size_t position)
{
  rec_rset_t rset;

  rset = NULL;

  if (db->size > 0)
    {
      if (position >= db->size)
        {
          position = db->size - 1;
        }

      rset = (rec_rset_t) gl_list_get_at (db->rset_list, position);
    }

  return rset;
}

bool
rec_db_insert_rset (rec_db_t db,
                    rec_rset_t rset,
                    size_t position)
{
  gl_list_node_t node;

  node = NULL;

  if (position == 0)
    {
      node = gl_list_nx_add_first (db->rset_list,
                                   (void *) rset);
    }
  else if (position >= db->size)
    {
      node = gl_list_nx_add_last (db->rset_list,
                                  (void *) rset);
    }
  else
    {
      node = gl_list_nx_add_at (db->rset_list,
                                position,
                                (void *) rset);
    }

  if (node != NULL)
    {
      db->size++;
      return true;
    }

  return false;
}

bool
rec_db_remove_rset (rec_db_t db, size_t position)
{
  bool removed;

  removed = false;
  
  if (db->size > 0)
    {
      if (position >= db->size)
        {
          position = db->size - 1;
        }
      
      if (gl_list_remove_at (db->rset_list,
                             position))
        {
          db->size--;
          removed = true;
        }
    }
  
  return removed;
}

bool
rec_db_type_p (rec_db_t db,
               const char *type)
{
  return (rec_db_get_rset_by_type (db, type) != NULL);
}

rec_rset_t
rec_db_get_rset_by_type (rec_db_t db,
                         const char *type)
{
  int i;
  rec_rset_t rset;
  bool found;
  char *rtype;

  found = false;
  for (i = 0; i < rec_db_size (db); i++)
    {
      rset = rec_db_get_rset (db, i);
      rtype = rec_rset_type (rset);
      if (rtype == NULL)
        {
          if (type == NULL)
            {
              /* Return the default rset.  */
              found = true;
              break;
            }
        }
      else
        { 
          if ((type != NULL)
              && (strcmp (rtype, type) == 0))
            {
              found = true;
              break;
            }
        }
    }

  if (!found)
    {
      rset = NULL;
    }
  
  return rset;
}

rec_rset_t
rec_db_query (rec_db_t     db,
              const char  *type,
              const char  *join,
              size_t       *index,
              rec_sex_t    sex,
              const char  *fast_string,
              size_t       random,
              rec_fex_t    fex,
              const char  *password,
              rec_fex_t    group_by,
              rec_fex_t    sort_by,
              int          flags)
{
  rec_rset_t res = NULL;
  rec_rset_t rset = NULL;

  /* Create a new, empty, record set, that will contain the contents
     of the selection.  */

  res = rec_rset_new ();
  if (!res)
    {
      /* Out of memory.  */
      return NULL;
    }

  /* Search for the rset containing records of the requested type.  If
     type equals to NULL then the default record set is used.  If JOIN
     is not NULL then the record set must be the join of the involved
     record sets.  */

  rset = rec_db_get_rset_by_type (db, type);
  if (!rset)
    {
      /* If the default record set was selected, it was not found, and
         the database contains only one record set, then it is
         selected.  */

      if (!type && (rec_db_size (db) == 1))
        {
          rset = rec_db_get_rset (db, 0);
        }
      else
        {
          /* Type not found, so return an empty record set.  */
          return res;
        }
    }
  else
    {
      if (join)
        {
          /* A join was requested.  The steps to proceed are:
             
             - Make sure that the requested field join is declared of
               type 'rec' in the record set.
             - Retrieve the referred record set from the database.
             - Calculate the join and store it in 'rset'.
          */

          rec_type_t ref_type = rec_rset_get_field_type (rset, join);
          if (ref_type && (rec_type_kind (ref_type) == REC_TYPE_REC))
            {
              const char *referred_type = rec_type_rec (ref_type);

              if (rec_db_get_rset_by_type (db, referred_type))
                {
                  rset = rec_db_join (db, type, join, referred_type);
                  if (!rset)
                    {
                      /* Out of memory.  */
                      return NULL;
                    }
                }
            }
        }
    }

  /* If a descriptor is requested then get a copy of the descriptor of
     the referred record set, which exists only if it is not the
     default.  */

  if (flags & REC_F_DESCRIPTOR)
    {
      rec_record_t descriptor = rec_rset_descriptor (rset);
      if (descriptor)
        {
          descriptor = rec_record_dup (descriptor);
          if (!descriptor)
            {
              /* Out of memory.  */
              free (res);
              return NULL;
            }
        }

      rec_rset_set_descriptor (res, descriptor);
    }
  
  /* Generate a list of random indexes here if requested.  The
     generated random indexes are added to the indexes list, which
     must be NULL if random > 0 (mutually exclusive arguments).  */

  if (random > 0)
    {
      rec_db_add_random_indexes (&index, random, rec_rset_num_records (rset));
      if (!index)
        {
          /* Out of memory.  */
          return NULL;
        }
    }

  if (fex && !group_by && rec_fex_all_calls_p (fex))
    {
      /* This query is a request for the value of several aggregates,
         with no grouping.  This means that the resulting rset will
         contain one record containing the evaluation of the
         aggregates.  This is peformed by invoking rec_db_process_fex
         with a NULL record.  */

      rec_record_t record = rec_db_process_fex (db, rset, NULL, fex);
      if (record)
        {
          rec_record_set_container (record, res);
          if (!rec_mset_append (rec_rset_mset (res),
                                MSET_RECORD,
                                (void *) record,
                                MSET_RECORD))
            {
              /* Out of memory.  */
              return NULL;
            }
        }
    }
  else
    {
      /* Process this record set.  This means that every record of this
         record set which is selected by some of the selection arguments
         (a sex, an index, a random selection or a "fast string") will
         be duplicated and added to the 'res' record set.  */

      rec_record_t record = NULL;
      size_t num_rec = -1;

      if (group_by)
        {
          if (!rec_rset_sort (rset, group_by))
            {
              /* Out of memory.  */
              return NULL;
            }

          if (!rec_rset_group (rset, group_by))
            {
              /* Out of memory.  */
              return NULL;
            }
        }

      if (!rec_rset_sort (rset, sort_by))
        {
          /* Out of memory.  */
          return NULL;
        }

      rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));
      while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &record, NULL))
        {
          rec_record_t res_record;
          num_rec++;
        
          /* Determine whether we must skip this record.  */
        
          if (!rec_db_record_selected_p (num_rec,
                                         record,
                                         index,
                                         sex,
                                         fast_string,
                                         flags & REC_F_ICASE))
            {
              continue;
            }
              
          /* Process this record.  */

          /* Transform the record through the field expression and add
             it to the result record set.  */
      
          res_record
            = rec_db_process_fex (db, rset, record, fex);

          if (!res_record)
            {
              /* Out of memory.  */
              return NULL;
            }

          /* Do not add empty records to the result record set.  */

          if (rec_record_num_elems (res_record) == 0)
            {
              continue;
            }

#if defined REC_CRYPT_SUPPORT

          /* Decrypt the confidential fields in the record if some
             of the fields are declared as "confidential", but only
             do that if the user provided a password.  Note that we
             use 'rset' instead of 'res' to cover cases where (flags
             & REC_F_DESCRIPTOR) == 0.  */
        
          if (password)
            {
              if (!rec_decrypt_record (rset, res_record, password))
                {
                  /* Out of memory.  */
                  return NULL;
                }
            }
#endif

          /* Remove duplicated fields if requested by the user.  */
        
          if (flags & REC_F_UNIQ)
            {
              rec_record_uniq (res_record);
            }
        
          /* Append.  */

          rec_record_set_container (res_record, res);
          if (!rec_mset_append (rec_rset_mset (res),
                                MSET_RECORD,
                                (void *) res_record,
                                MSET_RECORD))
            {
              /* Out of memory.  */
              return NULL;
            }
        
        }
      rec_mset_iterator_free (&iter);
    }

  return res;
}

bool
rec_db_insert (rec_db_t db,
               const char *type,
               size_t *index,
               rec_sex_t sex,
               const char *fast_string,
               size_t random,
               const char *password,
               rec_record_t record,
               int flags)
{
  /* Discard NULL or empty records.  */
  
  if (!record || (rec_record_num_fields (record) == 0))
    {
      return true;
    }

  /* Insert the record in the database.  */

  if (index || sex || fast_string || (random > 0))
    {
      /* Replace matching records with copies of RECORD.  */

      rec_rset_t rset = rec_db_get_rset_by_type (db, type);
      if (rset)
        {
          size_t num_rec = -1;

          /* If the user requested to replace random records,
             calculate them now for this record set.  */

          if (random > 0)
            {
              rec_db_add_random_indexes (&index, random, rec_rset_num_records (rset));
              if (!index)
                {
                  /* Out of memory.  */
                  return false;
                }
            }

          /* Add auto generated fields unless the user disabled
             it.  */

          if (!(flags & REC_F_NOAUTO))
            {
              if (!rec_rset_add_auto_fields (rset, record))
                {
                  /* Out of memory.  */
                  return false;
                }
            }

#if defined REC_CRYPT_SUPPORT

          /* Encrypt confidential fields if a password was provided by
             the user.  */

          if (password)
            {
              if (!rec_encrypt_record (rset, record, password))
                {
                  /* Out of memory.  */
                  return false;
                }
            }
#endif

          /* Iterate on the record set, replacing matching records
             with copies of the provided record.  */

          {
            rec_record_t rset_record = NULL;
            rec_mset_elem_t elem;
            rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));

            while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &rset_record, &elem))
              {
                num_rec++;

                /* Shall we skip this record?  */

                if (!rec_db_record_selected_p (num_rec,
                                               rset_record,
                                               index,
                                               sex,
                                               fast_string,
                                               flags & REC_F_ICASE))
                  {
                    continue;
                  }

                /* Replace the record.  */

                rec_record_set_container (record, rset);
                rec_mset_elem_set_data (elem, (void *) rec_record_dup (record));
                
              }
            rec_mset_iterator_free (&iter);
          }
        }
    }
  else
    {
      /* Append the record in the proper record set.  */
      
      rec_rset_t rset = rec_db_get_rset_by_type (db, type);

      if (rset)
        {
          rec_record_set_container (record, rset);

          /* Add auto-set fields required by this record set, unless
             the addition of auto-fields is disabled by the user.  */

          if (!(flags & REC_F_NOAUTO))
            {
              if (!rec_rset_add_auto_fields (rset, record))
                {
                  /* Out of memory.  */
                  return false;
                }
            }

#if defined REC_CRYPT_SUPPORT
          /* Encrypt confidential fields if a password was
             provided.  */

          if (password)
            {
              if (!rec_encrypt_record (rset, record, password))
                {
                  /* Out of memory.  */
                  return false;
                }
            }
#endif

          if (rec_rset_num_records (rset) == 0)
            {
              /* The rset is empty => Insert the new record just after
                 the relative position of the record descriptor.  */

              rec_mset_insert_at (rec_rset_mset (rset),
                                  MSET_RECORD,
                                  (void *) record,
                                  rec_rset_descriptor_pos (rset));
            }
          else
            {
              /* Insert the new record after the last record in the
                 set.  */

              rec_mset_t mset = rec_rset_mset (rset);
              rec_record_t last_record =
                (rec_record_t) rec_mset_get_at (mset,
                                                MSET_RECORD,
                                                rec_rset_num_records (rset) - 1);

              if (!rec_mset_insert_after (mset,
                                          MSET_RECORD,
                                          (void *) record,
                                          rec_mset_search (mset, (void *) last_record)))
                {
                  /* Out of memory.  */
                  return NULL;
                }
            }
        }
      else
        {
          /* Create a new type and insert the record there.  */

          rset = rec_rset_new ();
          if (!rset)
            {
              /* Out of memory.  */
              return false;
            }

          rec_rset_set_type (rset, type);
          rec_record_set_container (record, rset);
          if (!rec_mset_append (rec_rset_mset (rset),
                                MSET_RECORD,
                                (void *) record,
                                MSET_ANY))
            {
              /* Out of memory.  */
              return false;
            }

          if (type)
            {
              rec_db_insert_rset (db, rset, rec_db_size (db));
            }
          else
            {
              /* The default rset should always be placed in the
                 beginning of the db.  */

              rec_db_insert_rset (db, rset, 0);
            }
        }
    }

  return true;
}

bool
rec_db_delete (rec_db_t     db,
               const char  *type,
               size_t      *index,
               rec_sex_t    sex,
               const char  *fast_string,
               size_t       random,
               int          flags)
{
  /* Get the selected record set.  If the user did not specify a type,
     the default record set does not exist, and the database contains
     only one record set, then select it.  */

  rec_rset_t rset = rec_db_get_rset_by_type (db, type);
  if (!type && !rset && (rec_db_size (db) == 1))
    {
      rset = rec_db_get_rset (db, 0);
    }

  /* Don't process empty record sets.  */

  if (rec_rset_num_records (rset) == 0)
    {
      return true;
    }

  /* If the user requested to delete random records then calculate
     them now for this record set.  */

  if (random > 0)
    {
      rec_db_add_random_indexes (&index, random, rec_rset_num_records (rset));
      if (!index)
        {
          /* Out of memory.  */
          return false;
        }
    }

  /* Iterate on the records, deleting or commenting out the selected
       ones.  */

  {
    rec_record_t record = NULL;
    rec_mset_elem_t elem;
    size_t num_rec = -1;
    rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));

    while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &record, &elem))
      {
        num_rec++;

        if (!rec_db_record_selected_p (num_rec,
                                       record,
                                       index,
                                       sex,
                                       fast_string,
                                       flags & REC_F_ICASE))
          {
            continue;
          }

        if (flags & REC_F_COMMENT_OUT)
          {
            /* Replace the record with a comment in the current
               element.  */

            rec_comment_t comment = rec_record_to_comment (record);
            if (!comment)
              {
                /* Out of memory.  */
                return false;
              }

            rec_record_destroy (record);
            rec_mset_elem_set_data (elem, (void *) comment);
            rec_mset_elem_set_type (elem, MSET_COMMENT);
          }
        else
          {
            /* Remove the physical record from the record set and
               dispose it.  */

            rec_mset_remove_elem (rec_rset_mset (rset), elem);
          }
      }
    rec_mset_iterator_free (&iter);
  }

  return true;
}

bool rec_db_set (rec_db_t    db,
                 const char *type,
                 size_t     *index,
                 rec_sex_t   sex,
                 const char *fast_string,
                 size_t      random,
                 rec_fex_t   fex,
                 int         action,
                 const char *action_arg,
                 int         flags)
{
  /* Get the selected record set.  If the user did not specify a type,
     the default record set does not exist, and the database contains
     only one record set, then select it.  */

  rec_rset_t rset = rec_db_get_rset_by_type (db, type);
  if (!type && !rset && (rec_db_size (db) == 1))
    {
      rset = rec_db_get_rset (db, 0);
    }

  /* Don't process empty record sets.  */

  if (rec_rset_num_records (rset) == 0)
    {
      return true;
    }

  /* If the user requested to manipulate random records then calculate
     them now for this record set.  */

  if (random > 0)
    {
      rec_db_add_random_indexes (&index, random, rec_rset_num_records (rset));
      if (!index)
        {
          /* Out of memory.  */
          return false;
        }
    }
    
  /* Iterate on the records, operating on the selected ones.  */

  {
    rec_record_t record = NULL;
    size_t num_rec = -1;
    rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));
    bool descriptor_renamed = false;

    while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &record, NULL))
      {
        num_rec++;

        if (!rec_db_record_selected_p (num_rec,
                                       record,
                                       index,
                                       sex,
                                       fast_string,
                                       flags & REC_F_ICASE))
          {
            continue;
          }

        switch (action)
          {
          case REC_SET_ACT_RENAME:
            {
              /* If the operation is applied to all records of a given
                 type (or default) then change the record descriptor
                 as well.  But make sure to do it just once!  */

              bool rename_descriptor = false;
              if (!descriptor_renamed
                  && (sex == NULL) && (index == NULL) && (random == 0) && (fast_string == NULL))
                {
                  rename_descriptor = true;
                  descriptor_renamed = true;
                }

              if (!rec_db_set_act_rename (rset, record, fex, rename_descriptor, action_arg))
                {
                  /* Out of memory.  */
                  return false;
                }

              break;
            }
          case REC_SET_ACT_SET:
            {
              if (!rec_db_set_act_set (rset, record, fex, false, action_arg))
                {
                  /* Out of memory.  */
                  return false;
                }
              break;
            }
          case REC_SET_ACT_ADD:
            {
              if (!rec_db_set_act_add (rset, record, fex, action_arg))
                {
                  /* Out of memory.  */
                  return false;
                }
              break;
            }
          case REC_SET_ACT_SETADD:
            {
              if (!rec_db_set_act_set (rset, record, fex, true, action_arg))
                {
                  /* Out of memory.  */
                  return false;
                }
              break;
            }
          case REC_SET_ACT_DELETE:
            {
              if (!rec_db_set_act_delete (rset, record, fex, false))
                {
                  /* Out of memory.  */
                  return false;
                }
              break;
            }
          case REC_SET_ACT_COMMENT:
            {
              if (!rec_db_set_act_delete (rset, record, fex, true))
                {
                  /* Out of memory.  */
                  return false;
                }
              break;
            }
          default:
            {
              /* Ignore an invalid action.  */
              return true;
            }
          }
      }
    rec_mset_iterator_free (&iter);
  }

  return true;
}

rec_aggregate_reg_t
rec_db_aggregates (rec_db_t db)
{
  return db->aggregates;
}

/*
 * Private functions.
 */

static rec_record_t
rec_db_merge_records (rec_record_t record1,
                      rec_record_t record2,
                      const char *prefix)
{
  rec_mset_iterator_t iter;
  rec_field_t field;
  rec_record_t merge = NULL;

  merge = rec_record_dup (record1);
  if (!merge)
    {
      return NULL;
    }

  /* Add all the fields from record2 to record1, prepending PREFIX_ to
     the field name.  It is the responsability of the user to provide
     a PREFIX whose application results in a unique field.  */

  iter = rec_mset_iterator (rec_record_mset (record2));
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
    {
      rec_field_t new_field = rec_field_dup (field);
      if (!new_field)
        {
          /* Out of memory.  */
          return NULL;
        }

      /* Apply the prefix.  */
      {
        const char *field_name = rec_field_name (new_field);
        char *new_name = malloc (strlen (field_name) + strlen(prefix) + 2);
        if (!new_name)
          {
            /* Out of memory.  */
            return NULL;
          }

        memcpy (new_name, prefix, strlen (prefix));
        new_name[strlen (prefix)] = '_';
        memcpy (new_name + strlen (prefix) + 1, field_name, strlen (field_name) + 1);

        if (!rec_field_set_name (new_field, new_name))
          {
            /* Out of memory.  */
            return NULL;
          }

        free (new_name);
      }

      if (!rec_mset_append (rec_record_mset (merge),
                            MSET_FIELD,
                            (void *) new_field,
                            MSET_ANY))
        {
          /* Out of memory.  */
          return NULL;
        }
    }
  rec_mset_iterator_free (&iter);
  
  return merge;
}

static rec_rset_t
rec_db_join (rec_db_t db,
             const char *type1,
             const char *field,
             const char *type2)
{
  /* Note that this function is inefficient like hell.  */

  /* Perform the join of the specified record sets, using TYPE1.Field
     = TYPE2.Key as the join criteria.  If some of the specified
     record sets don't exist as named rset in the specified database
     then return NULL.  */

  const char *key  = NULL;
  rec_rset_t join  = NULL;
  rec_rset_t rset1 = rec_db_get_rset_by_type (db, type1);
  rec_rset_t rset2 = rec_db_get_rset_by_type (db, type2);
  

  if (!rset1 || !rset2)
    {
      return NULL;
    }

  /* Determine the key field of the second record set.  */
  key = rec_rset_key (rset2);
  if (!key)
    {
      return NULL;
    }

  /* Do the join.  */

  join = rec_rset_new ();
  if (!join)
    {
      /* Out of memory.  */
      return NULL;
    }
  else
    {
      rec_record_t record1 = NULL;
      rec_mset_iterator_t iter1 = rec_mset_iterator (rec_rset_mset (rset1));
      while (rec_mset_iterator_next (&iter1, MSET_RECORD, (const void **) &record1, NULL))
        {
          /* For each field record1.key in this record we scan the
             second record set for record1.field == record2.key
             instances.  */

          size_t num_foreign_keys = rec_record_get_num_fields_by_name (record1, field);
          size_t num_foreign_key = 0;

          for (num_foreign_key = 0; num_foreign_key < num_foreign_keys; num_foreign_key++)
            {
              rec_record_t record2 = NULL;
              rec_mset_iterator_t iter2 = rec_mset_iterator (rec_rset_mset (rset2));

              while (rec_mset_iterator_next (&iter2, MSET_RECORD, (const void **) &record2, NULL))
                {

                  /* Continue only if there is a field in record1 such as:
                     record1.field == record2.key.  */
                  
                  bool found = false;
                  size_t i = 0;
                  
                  rec_field_t key_field = rec_record_get_field_by_name (record2, key, num_foreign_key);
                  if (!key_field)
                    {
                      /* A record without a key is an integrity error, but
                         none of our business, so just skip it.  */
                      break;
                    }
                  
                  found = false;
                  for (i = 0; i < rec_record_get_num_fields_by_name (record1, field); i++)
                    {
                      if (strcmp (rec_field_value (key_field),
                                  rec_field_value (rec_record_get_field_by_name (record1, field, i))) == 0)
                        {
                          found = true;
                          break;
                        }
                    }
                  
                  if (!found)
                    {
                      /* Skip this combination record.  */
                      continue;
                    }
                  
                  /* Merge record1 and record2 into a new record.  */
                  
                  rec_record_t record = rec_db_merge_records (record1, record2, field);
                  if (!record)
                    {
                      /* Out of memory.  */
                      return NULL;
                    }
                  
                  /* Remove all the occurrences of the 'field' from
                     record1, which were substituted in the merge.  */
                  
                  while (rec_record_get_num_fields_by_name (record, field) > 0)
                    {
                      rec_record_remove_field_by_name (record, field, 0);
                    }
                  
                  /* Add it into the join result.  */
                  
                  rec_record_set_container (record, join);
                  if (!rec_mset_append (rec_rset_mset (join), MSET_RECORD, (void *) record, MSET_ANY))
                    {
                      /* Out of memory.  */
                      return NULL;
                    }
                }
              rec_mset_iterator_free (&iter2);
            }
        }
      rec_mset_iterator_free (&iter1);
    }

  /* The descriptor of the new record set will define records of type
     TYPE_FIELD, where FIELD is the name specified to trigger the
     operation.  The contents of the descriptor will be just the
     %rec entry. */

  {
    rec_record_t new_descriptor = rec_record_new ();
    if (!new_descriptor)
      {
        /* Out of memory.  */
        return NULL;
      }

    /* Set the type of the joined record set.  */

    {
      rec_field_t new_field = NULL;
      char *new_rset_type = rec_concat_strings (type1, "_", field);
      if (!new_rset_type)
        {
          /* Out of memory.  */
          return NULL;
        }

      new_field = rec_field_new (rec_std_field_name (REC_FIELD_REC),
                                 new_rset_type);
      if (!rec_mset_append (rec_record_mset (new_descriptor),
                            MSET_FIELD,
                            (void *) new_field,
                            MSET_ANY))
        {
          /* Out of memory.  */
          return NULL;
        }
    }

    rec_rset_set_descriptor (join, new_descriptor);
  }

  return join;
}

static bool
rec_db_set_act_rename (rec_rset_t rset,
                       rec_record_t record,
                       rec_fex_t fex,
                       bool rename_descriptor,
                       const char *arg)
{
  size_t j, min, max, renamed;
  size_t num_fields;
  rec_fex_elem_t fex_elem;
  rec_field_t field;
  const char *field_name;

  /* Rename the selected fields.  The size of the FEX is guaranteed to
     be 1 at this point.  */
 
  fex_elem = rec_fex_get (fex, 0);
  field_name = rec_fex_elem_field_name (fex_elem);
  min = rec_fex_elem_min (fex_elem);
  max = rec_fex_elem_max (fex_elem);

  num_fields =
    rec_record_get_num_fields_by_name (record, field_name);
  if (min == -1)
    {
      /* Process all the fields with the given name.  */
      min = 0;
      max = num_fields - 1;
    }
  if (max == -1)
    {
      max = min;
    }

  renamed = 0;
  for (j = 0; j < num_fields; j++)
    {
      if ((j >= min) && (j <= max))
        {
          /* Set the name of the Jth field
             named FIELD_NAME, if it exists.*/
          field = rec_record_get_field_by_name (record,
                                                field_name,
                                                j - renamed);
          if (field)
            {
              rec_field_set_name (field, arg);
              renamed++;
            }

          if (rename_descriptor)
            {
              
              rec_rset_rename_field (rset,
                                     field_name,
                                     arg);
            }
        }
    }

  return true;
}


static bool
rec_db_set_act_set (rec_rset_t rset,
                    rec_record_t record,
                    rec_fex_t fex,
                    bool add_p,
                    const char *arg)
{
  size_t i, j, min, max;
  size_t num_fields;
  rec_fex_elem_t fex_elem;
  rec_field_t field;
  const char *field_name;

  for (i = 0; i < rec_fex_size (fex); i++)
    {
      fex_elem = rec_fex_get (fex, i);
      field_name = rec_fex_elem_field_name (fex_elem);
      min = rec_fex_elem_min (fex_elem);
      max = rec_fex_elem_max (fex_elem);
      
      num_fields =
        rec_record_get_num_fields_by_name (record, field_name);
      if (min == -1)
        {
          /* Process all the fields with the given name.  */
          min = 0;
          max = num_fields - 1;
        }
      if (max == -1)
        {
          max = min;
        }
      
      for (j = 0; j < num_fields; j++)
        {
          if ((j >= min) && (j <= max))
            {
              /* Set the value of the Jth field
                 named FIELD_NAME, if it exists.*/
              field = rec_record_get_field_by_name (record,
                                                    field_name,
                                                    j);
              if (field)
                {
                  rec_field_set_value (field, arg);
                }
            }
        }

      if (add_p && (num_fields == 0))
        {
          /* Add a field with this name and value.  */
          field = rec_field_new (field_name, arg);
          if (!rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY))
            {
              /* Out of memory.  */
              return false;
            }
        }
    }

  return true;
}

static bool
rec_db_set_act_add (rec_rset_t rset,
                    rec_record_t record,
                    rec_fex_t fex,
                    const char *arg)
{
  size_t i;

  /* Create new fields from the FEX and add them to the record.  */
  for (i = 0; i < rec_fex_size (fex); i++)
    {
      rec_fex_elem_t fex_elem = rec_fex_get (fex, i);
      const char *field_name = rec_fex_elem_field_name (fex_elem);
      rec_field_t field = rec_field_new (field_name, arg);
      if (!field)
        {
          /* Out of memory.  */
          return false;
        }

      if (!rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY))
        {
          /* Out of memory.  */
          return false;
        }
    }

  return true;
}

static bool
rec_db_set_act_delete (rec_rset_t rset,
                       rec_record_t record,
                       rec_fex_t fex,
                       bool comment_out)
{
  size_t i, j;
  size_t num_fields;
  bool *deletion_mask;
  rec_field_t field;
  rec_mset_iterator_t iter;
  rec_mset_elem_t elem;

  /* Initialize the deletion mask.  */
  deletion_mask = malloc (sizeof (bool) * rec_record_num_fields (record));
  if (!deletion_mask)
    {
      /* Out of memory.  */
      return false;
    }

  for (i = 0; i < rec_record_num_fields (record); i++)
    {
      deletion_mask[i] = false;
    }
                    
  /* Mark fields that will be deleted from the record.  */
  for (i = 0; i < rec_fex_size (fex); i++)
    {
      rec_fex_elem_t fex_elem = rec_fex_get (fex, i);
      const char *field_name = rec_fex_elem_field_name (fex_elem);
      size_t min = rec_fex_elem_min (fex_elem);
      size_t max = rec_fex_elem_max (fex_elem);

      num_fields =
        rec_record_get_num_fields_by_name (record, field_name);
      if (min == -1)
        {
          /* Delete all the fields with the given name.  */
          min = 0;
          max = num_fields - 1;
        }
      if (max == -1)
        {
          max = min;
        }

      for (j = 0; j < num_fields; j++)
        {
          if ((j >= min) && (j <= max))
            {
              /* Mark this field for deletion.  */
              field = rec_record_get_field_by_name (record,
                                                    rec_fex_elem_field_name (fex_elem),
                                                    j);
              deletion_mask[rec_record_get_field_index (record, field)] = true;
            }
        }
    }
                    
  /* Delete the marked fields.  */
  i = 0;

  iter = rec_mset_iterator (rec_record_mset (record));
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void**) &field, &elem))
    {
      if (deletion_mask[i])
        {
          if (comment_out)
            {
              /* Turn the field into a comment.  */

              rec_comment_t comment = rec_field_to_comment (field);
              if (!comment)
                {
                  /* Out of memory.  */
                  return false;
                }

              rec_field_destroy (field);
              rec_mset_elem_set_data (elem, (void *) comment);
              rec_mset_elem_set_type (elem, MSET_COMMENT);
            }
          else
            {
              /* Remove the field from the list and dispose it.  */
              
              rec_mset_remove_elem (rec_record_mset (record), elem);
            }
        }

      i++;
    }
  rec_mset_iterator_free (&iter);

  return true;
}

static bool
rec_db_index_p (size_t *index,
                size_t num)
{
  while ((index[0] != REC_Q_NOINDEX) || (index[1] != REC_Q_NOINDEX))
    {
      bool found = false;
      size_t min = index[0];
      size_t max = index[1];
      
      if (max == REC_Q_NOINDEX)
        {
          found = (num == min);
        }
      else
        {
          found = ((num >= min) && (num <= max));
        }
      
      if (found)
        {
          return true;
        }
      
      index = index + 2;
    }

  return false;
}

static void
rec_db_add_random_indexes (size_t **index,
                           size_t num,
                           size_t limit)
{
  /* Create NUM different random numbers in the [0..limit-1] range,
     without repetition, and store them in a buffer pointed by
     INDEX.  */

  size_t i;
  char random_state[128];
  struct random_data random_data;

  *index = malloc (sizeof(size_t) * ((num + 1) * 2));
  if (*index == NULL)
    {
      /* Out of memory.  */
      return;
    }
  
  for (i = 0; i < ((num + 1) * 2); i++)
    {
      (*index)[i]   = REC_Q_NOINDEX;
    }

  /* Insert the random indexes.  */

  memset (&random_data, 0, sizeof (random_data));
  initstate_r (time(NULL), (char *) &random_state, 128, &random_data);
  for (i = 0; i < (num * 2); i = i + 2)
    {
      size_t random_value = 0;
      
      random_r (&random_data, (int32_t *) &random_value); /* Can't fail.  */
      random_value = random_value % limit;

      /* Avoid having repeated random indexes.  */
      
      if (rec_db_index_p (*index, random_value))
        {
          /* Pick the first available number.  */

          size_t i;
          for (i = 0; i < limit; i++)
            {
              if (!rec_db_index_p (*index, i))
                {
                  random_value = i;
                  break;
                }
            }
        }

      (*index)[i]   = random_value;  /* Min.  */
      (*index)[i+1] = REC_Q_NOINDEX; /* Max.  */
    }
}

static bool
rec_db_record_selected_p (size_t num_record,
                          rec_record_t record,
                          size_t *index,
                          rec_sex_t sex,
                          const char *fast_string,
                          bool case_insensitive_p)
{
  /* Note that the logic in this function assumes that SEX and
     FAST_STRING are mutually exclusive.  If they are not then
     FAST_STRING takes precedence.  */

  /* The record is searched for instances of the "fast string", which
     can appear as a substring.  */

  if (fast_string)
    {
      return rec_record_contains_value (record,
                                        fast_string,
                                        case_insensitive_p);
    }

  /* The selected expression is evaluated in the record.  If there is
     an error evaluating the expression, or if the expression does not
     evaluate to 'true', then 'false' is returned.  */

  if (sex)
    {
      bool eval_status;
      return rec_sex_eval (sex, record, &eval_status);
    }

  /* The memory pointed by INDEX contains pairs of indexes Min,Max.
     The final pair is always REC_Q_NOINDEX,REC_Q_NOINDEX.  Select the
     current record only if its position is into some of the defined
     intervals.  */
  
  if (index)
    {
      return rec_db_index_p (index, num_record);
    }

  return true;
}

static rec_record_t
rec_db_process_fex (rec_db_t db,
                    rec_rset_t rset,
                    rec_record_t record,
                    rec_fex_t fex)
{
  rec_record_t res = NULL;
  size_t fex_size, i, j = 0;

  /* If fex is NULL then just return a copy of RECORD.  Otherwise
     create an empty result record.  */
  
  if (!fex)
    {
      return rec_record_dup (record);
    }

  res = rec_record_new ();
  if (!res)
    {
      /* Out of memory.  */
      return NULL;
    }

  /* Iterate on the elements of the FEX, picking and transforming the
     fields of RECORD that must be copied and inserted into RES.  If a
     function call is found in the fex then invoke the corresponding
     function and add the fields returned by that function into the
     record.  */

  fex_size = rec_fex_size (fex);
  for (i = 0; i < fex_size; i++)
    {
      rec_fex_elem_t elem = rec_fex_get (fex, i);
      const char *field_name = rec_fex_elem_field_name (elem);
      const char *alias = rec_fex_elem_rewrite_to (elem);
      const char *function_name = rec_fex_elem_function_name (elem);
      size_t min = rec_fex_elem_min (elem);
      size_t max = rec_fex_elem_max (elem);

      if (function_name)
        {
          /* Get a handler for the aggregate function and invoke it on
             the rset or record, passing the field_name argument and
             the indexes.  The value returned by the funciton is then
             appended into the current record in a new field, named
             after the name of the aggregate and the name of the
             argument field.  Non-existing aggregates are simply
             ignored.  */

          rec_aggregate_t func = rec_aggregate_reg_get (rec_db_aggregates (db), function_name);
          if (func)
            {
              char *func_res = (func) (rset, record, field_name);
              if (func_res)
                {
                  /* Add a new field with the result of the aggregate
                     as its value.  */

                  rec_field_t agg_field = NULL;
                  char *agg_field_name = NULL;
                  char *agg_field_value = func_res;

                  /* The name of the new field is a composition of the
                     name of the invoked function and the name of the
                     field to which the function is applied.  Unless
                     an alias is used, of course.  */

                  if (alias)
                    {
                      agg_field_name = strdup (alias);
                      if (!agg_field_name)
                        {
                          /* Out of memory.  */
                          return NULL;
                        }
                    }
                  else
                    {
                      agg_field_name = malloc (strlen(function_name) + 1 /* _ */ + strlen (field_name) + 1);
                      if (!agg_field_name)
                        {
                          /* Out of memory.  */
                          return NULL;
                        }
                      
                      memcpy (agg_field_name, function_name, strlen (function_name) + 1);
                      strcat (agg_field_name, "_");
                      strcat (agg_field_name, field_name);
                    }

                  agg_field = rec_field_new (agg_field_name, agg_field_value);
                  if (!agg_field)
                    {
                      /* Out of memory.  */
                      return NULL;
                    }
                  
                  if (!rec_mset_append (rec_record_mset (res),
                                        MSET_FIELD,
                                        (void *) agg_field,
                                        MSET_FIELD))
                    {
                      /* Out of memory.  */
                      return NULL;
                    }

                  free (agg_field_name);
                  free (func_res);
                }
            }
        }
      else
        {
          if ((min == -1) && (max == -1))
            {
              /* Add all the fields with that name.  */
              min = 0;
              max = rec_record_get_num_fields_by_name (record, field_name);
            }
          else if (max == -1)
            {
              /* Add just one field: Field[min].  */
              max = min + 1;
            }
          else
            {
              /* Add the interval min..max, max inclusive.  */
              max++;
            }

          /* Add the selected fields to the result record.  */

          for (j = min; j < max; j++)
            {
              rec_field_t res_field = NULL;
              rec_field_t field =
                rec_record_get_field_by_name (record, field_name, j);

              if (!field)
                {
                  continue;
                }

              /* Duplicate the field and append it into 'res'.  If there
                 is a rewrite rule defined in this fex entry then use it
                 instead of the original name of the field.  */

              res_field = rec_field_dup (field);
              if (alias)
                {
                  if (!rec_field_set_name (res_field, alias))
                    {
                      /* Out of memory.  */
                      return NULL;
                    }
                }

              if (!rec_mset_append (rec_record_mset (res),
                                    MSET_FIELD,
                                    (void *) res_field,
                                    MSET_FIELD))
                {
                  /* Out of memory.  */
                  return NULL;
                }
            }
        }
    }

  /* At this point RES is a record containing all the selected fields
     of the original record, but we must also copy the location
     information.  */

  if (record)
    {
      rec_record_set_location (res, rec_record_location (record));
      rec_record_set_char_location (res, rec_record_char_location (record));
    }

  return res;
}

static bool
rec_db_rset_equals_fn (const void *elt1,
                       const void *elt2)
{
  return false;
}

static void
rec_db_rset_dispose_fn (const void *elt)
{
  rec_rset_t rset;

  rset = (rec_rset_t) elt;
  rec_rset_destroy (rset);
}

/* End of rec-db.c */
