/* -*- mode: C -*-
 *
 *       File:         rec-int.c
 *       Date:         Thu Jul 15 18:23:26 2010
 *
 *       GNU recutils - Data integrity.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <gettext.h>
#define _(str) dgettext (PACKAGE, str)
#include <tempname.h>

#if defined REMOTE_DESCRIPTORS
#   include <curl/curl.h>
#endif

#include <rec.h>
#include <rec-utils.h>

/*
 * Forward references.
 */

static int rec_int_check_descriptor (rec_rset_t rset, rec_buf_t errors);
static int rec_int_check_record_key (rec_rset_t rset,
                                     rec_record_t orig_record, rec_record_t record,
                                     rec_buf_t errors);
static int rec_int_check_record_types (rec_db_t db,
                                       rec_rset_t rset,
                                       rec_record_t record,
                                       rec_buf_t errors);
static int rec_int_check_record_mandatory (rec_rset_t rset, rec_record_t record,
                                           rec_buf_t errors);
static int rec_int_check_record_unique (rec_rset_t rset, rec_record_t record,
                                        rec_buf_t errors);
static int rec_int_check_record_prohibit (rec_rset_t rset, rec_record_t record,
                                          rec_buf_t errors);
static int rec_int_check_record_sex_constraints (rec_rset_t rset, rec_record_t record,
                                                 rec_buf_t errors);
static int rec_int_check_record_allowed (rec_rset_t rset, rec_record_t record,
                                         rec_buf_t errors);

#if defined REC_CRYPT_SUPPORT
static int rec_int_check_record_secrets (rec_rset_t rset, rec_record_t record,
                                         rec_buf_t errors);
#endif

static int rec_int_merge_remote (rec_rset_t rset, rec_buf_t errors);
static bool rec_int_rec_type_p (const char *str);

/* The following macros are used by some functions in this file to
   reduce verbosity.  */

#define FNAME(id) rec_std_field_name ((id))

#define ADD_ERROR(buf,str,...)                          \
  do                                                    \
    {                                                   \
      char *tmp = NULL;                                 \
      if (asprintf (&tmp, (str), __VA_ARGS__) != -1)    \
        {                                               \
          rec_buf_puts (tmp, (buf));                    \
          free (tmp);                                   \
        }                                               \
    }                                                   \
  while (0)

/*
 * Public functions.
 */

int
rec_int_check_db (rec_db_t db,
                  bool check_descriptors_p,
                  bool remote_descriptors_p,
                  rec_buf_t errors)
{
  int ret;
  size_t db_size;
  size_t n_rset;
  rec_rset_t rset;
  
  ret = 0;

  db_size = rec_db_size (db);
  for (n_rset = 0; n_rset < db_size; n_rset++)
    {
      rset = rec_db_get_rset (db, n_rset);
      ret = ret + rec_int_check_rset (db,
                                      rset,
                                      check_descriptors_p,
                                      remote_descriptors_p,
                                      errors);
    }

  return ret;
}

int
rec_int_check_rset (rec_db_t db,
                    rec_rset_t rset,
                    bool check_descriptor_p,
                    bool remote_descriptor_p,
                    rec_buf_t errors)
{
  int res;
  rec_mset_iterator_t iter;
  rec_record_t record;
  rec_record_t descriptor;
  size_t num_records, min_records, max_records;

  res = 0;

  if (remote_descriptor_p
      && (descriptor = rec_rset_descriptor (rset)))
    {
      /* Make a backup of the record descriptor to restore it
         later.  */
      descriptor = rec_record_dup (descriptor);

      /* Fetch the remote descriptor, if any, and merge it with the
         local descriptor.  If there is any error, stop and report
         it.  */
      res = rec_int_merge_remote (rset, errors);
      if (res > 0)
        {
          return res;
        }
    }

  if (check_descriptor_p)
    {
      res += rec_int_check_descriptor (rset, errors);
    }

  if (res > 0)
    {
      /* Stop here, since a lot of errors in the records will be
         generated due to errors in the record descriptor.  */
      return res;
    }

  /* Verify rset size restrictions.  */
  num_records = rec_rset_num_records (rset);
  min_records = rec_rset_min_records (rset);
  max_records = rec_rset_max_records (rset);

  if (min_records == max_records)
    {
      if (num_records != min_records)
        {
          ADD_ERROR (errors,
                     _("%s: error: the number of records of type %s should be %zd.\n"),
                     rec_rset_source (rset), rec_rset_type (rset), min_records);
          res++;
        }
    }
  else
    {
      if (num_records > rec_rset_max_records (rset))
        {
          ADD_ERROR (errors,
                     _("%s: error: too many records of type %s. Maximum allowed are %zd.\n"),
                     rec_rset_source (rset), rec_rset_type (rset), rec_rset_max_records (rset));
          res++;
        }
      if (num_records < rec_rset_min_records (rset))
        {
          ADD_ERROR (errors,
                     _("%s: error: too few records of type %s. Minimum allowed are %zd.\n"),
                    rec_rset_source (rset), rec_rset_type (rset), rec_rset_min_records (rset));
          res++;
        }
    }
  
  iter = rec_mset_iterator (rec_rset_mset (rset));
  while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &record, NULL))
    {
      res += rec_int_check_record (db,
                                   rset,
                                   record, record,
                                   errors);
    }

  rec_mset_iterator_free (&iter);

  if (remote_descriptor_p)
    {
      /* Restore the original descriptor in the record set.  */
      rec_rset_set_descriptor (rset, descriptor);
    }

  return res;
}

int
rec_int_check_record (rec_db_t db,
                      rec_rset_t rset,
                      rec_record_t orig_record,
                      rec_record_t record,
                      rec_buf_t errors)
{
  int res;

  res =
    rec_int_check_record_key (rset, orig_record, record, errors)
    + rec_int_check_record_types     (db, rset, record, errors)
    + rec_int_check_record_mandatory (rset, record, errors)
    + rec_int_check_record_unique    (rset, record, errors)
#if defined REC_CRYPT_SUPPORT
    + rec_int_check_record_secrets   (rset, record, errors)
#endif
    + rec_int_check_record_prohibit  (rset, record, errors)
    + rec_int_check_record_sex_constraints (rset, record, errors)
    + rec_int_check_record_allowed   (rset, record, errors);

  return res;
}

bool
rec_int_check_field_type (rec_db_t db,
                          rec_rset_t rset,
                          rec_field_t field,
                          rec_buf_t errors)
{
  bool res = true;
  rec_type_t type;
  char *errors_str;

  res = true;


  /* Get the proper type to check 'field' with, checking with the type
     from the type registry of 'rset', if any.  */

  type = rec_rset_get_field_type (rset, rec_field_name (field));

  /* Check the field with the type.  This is done by simply invoking
     rec_type_check on the field value.  An exception to this is the
     'rec' type.  The 'rec' type is used to implement foreign keys,
     and its effect on the type integrity system is that the value of
     the field must be considered to be of whatever type the primary
     key of the referred record set is.  */

  if (type)
    {
      if (rec_type_kind (type) == REC_TYPE_REC)
        {
          /* Get the name of the referred record set.  Check the type
             if and only if:
             
             - The referred rset exists in DB and
             - The referred rset has a primary key.
             - The primary key of the referred rset has a type.
          */

          const char *rset_type = rec_type_rec (type);
          rec_rset_t rset = rec_db_get_rset_by_type (db, rset_type);

          if (rset)
            {
              const char *key = rec_rset_key (rset);
              rec_type_t key_type = rec_rset_get_field_type (rset, key);

              if (key_type)
                {
                  if (!rec_type_check (key_type, rec_field_value (field), &errors_str))
                    {
                      if (errors)
                        {
                          ADD_ERROR (errors,
                                     "%s:%s: error: %s\n",
                                     rec_field_source (field), rec_field_location_str (field),
                                     errors_str);
                        }
                      free (errors_str);
                      res = false;
                    }
                }
            }
        }
      else
        {
          if (!rec_type_check (type, rec_field_value (field), &errors_str))
            {
              if (errors)
                {
                  ADD_ERROR (errors,
                             "%s:%s: error: %s\n",
                             rec_field_source (field), rec_field_location_str (field),
                             errors_str);
                }
              free (errors_str);
              res = false;
            }
        }
    }

  return res;
}

/*
 * Private functions
 */

static rec_fex_t
rec_int_collect_field_list (rec_record_t record,
                            const char *fname)
{
  size_t i, j = 0;
  size_t num_fields = rec_record_get_num_fields_by_name (record, fname);
  rec_fex_t res = rec_fex_new (NULL, REC_FEX_SIMPLE);

  if (!res)
    return NULL; /* Out of memory.  */

  for (i = 0; i < num_fields; i++)
    {
      rec_field_t field = rec_record_get_field_by_name (record, fname, i);
      rec_fex_t fex = rec_fex_new (rec_field_value (field), REC_FEX_SIMPLE);
      if (!fex)
        /* Invalid value in the field.  Ignore it.  */
        continue;

      for (j = 0; j < rec_fex_size (fex); j++)
        {
          rec_fex_elem_t elem = rec_fex_get (fex, j);
          char *field_name = strdup (rec_fex_elem_field_name (elem));

          if (!field_name
              || !rec_fex_append (res,
                                  field_name,
                                  rec_fex_elem_min (elem),
                                  rec_fex_elem_max (elem)))
            /* Not enough memory: panic and retreat!  */
            return NULL;
        }
      rec_fex_destroy (fex);
    }

  return res;
}

static int
rec_int_check_record_types (rec_db_t db,
                            rec_rset_t rset,
                            rec_record_t record,
                            rec_buf_t errors)
{
  int res;
  rec_field_t field;
  rec_mset_iterator_t iter;

  res = 0;

  iter = rec_mset_iterator (rec_record_mset (record));
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
    {
      /* Check for the type.  */
      if (!rec_int_check_field_type (db, rset, field, errors))
        {
          res++;
        }
    }

  rec_mset_iterator_free (&iter);

  return res;
}

static int
rec_int_check_record_mandatory (rec_rset_t rset,
                                rec_record_t record,
                                rec_buf_t errors)
{
  rec_fex_t fex_mandatory = NULL;
  int res = 0;
  size_t i;
  
  rec_record_t descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      fex_mandatory = rec_int_collect_field_list (descriptor, FNAME(REC_FIELD_MANDATORY));
      if (!fex_mandatory)
        {
          ADD_ERROR (errors, _("out of memory\n"), "");
          res = 1;
          goto cleanup;
        }

      /* Make sure that all fields in the mandatory fields list are in
         this record.  */

      for (i = 0; i < rec_fex_size (fex_mandatory); i++)
        {
          const char *fname = rec_fex_elem_field_name (rec_fex_get (fex_mandatory, i));
          if (rec_record_get_num_fields_by_name (record, fname)
              == 0)
            {
              ADD_ERROR (errors,
                         _("%s:%s: error: mandatory field '%s' not found in record\n"),
                         rec_record_source (record),
                         rec_record_location_str (record),
                         fname);
              res++;
            }
        }
    }

 cleanup:

  rec_fex_destroy (fex_mandatory);
  return res;
}

static int
rec_int_check_record_allowed (rec_rset_t rset,
                              rec_record_t record,
                              rec_buf_t errors)
{
  /* If %allowed is specified then fields with names not in the union
     of %allowed + %mandatory + %key are not allowed in records, and
     thus that situation is an integrity error.  */

  rec_fex_t fex_allowed   = NULL;
  rec_fex_t fex_mandatory = NULL;
  rec_fex_t fex_key       = NULL;
  
  int res = 0;
  rec_record_t descriptor = rec_rset_descriptor (rset);

  if (descriptor)
    {
      fex_allowed = rec_int_collect_field_list (descriptor,   FNAME(REC_FIELD_ALLOWED));
      fex_mandatory = rec_int_collect_field_list (descriptor, FNAME(REC_FIELD_MANDATORY));
      fex_key = rec_int_collect_field_list (descriptor,       FNAME(REC_FIELD_KEY));

      if (!fex_allowed || !fex_mandatory || !fex_key)
        {
          ADD_ERROR (errors, _("out of memory\n"), "");
          res = 1;
          goto cleanup;
        }


      if (rec_fex_size (fex_allowed) == 0)
        /* Nothing to do.  */
        goto cleanup;

      /* Make sure that all the fields in RECORD are in either
         %allowed, %mandatory or %key.  */
          
      rec_field_t field = NULL;
      rec_mset_iterator_t iter = rec_mset_iterator (rec_record_mset (record));
      while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
        {
          const char *field_name = rec_field_name (field);
          if (!(rec_fex_member_p (fex_allowed, field_name, -1, -1)
                || rec_fex_member_p (fex_mandatory, field_name, -1, -1)
                || rec_fex_member_p (fex_key, field_name, -1, -1)))
            {
              /* This field is not allowed.  */
              ADD_ERROR (errors,
                         _("%s:%s: error: field '%s' not allowed in this record set\n"),
                         rec_record_source (record),
                         rec_record_location_str (record),
                         field_name);
              res++;
            }
        }
      rec_mset_iterator_free (&iter);
    }
  
 cleanup:
  
  rec_fex_destroy (fex_allowed);
  rec_fex_destroy (fex_mandatory);
  rec_fex_destroy (fex_key);
  return res;
}

static int
rec_int_check_record_unique (rec_rset_t rset,
                             rec_record_t record,
                             rec_buf_t errors)
{
  rec_fex_t fex_unique = NULL;
  int res = 0;
  size_t i;

  rec_record_t descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      fex_unique = rec_int_collect_field_list (descriptor, FNAME(REC_FIELD_UNIQUE));
      if (!fex_unique)
        {
          ADD_ERROR (errors, _("out of memory\n"), "");
          res = 1;
          goto cleanup;
        }

      /* Make sure that all fields in the unique fields list are
         unique in this record.  */

      for (i = 0; i < rec_fex_size (fex_unique); i++)
        {
          const char *fname = rec_fex_elem_field_name (rec_fex_get (fex_unique, i));
          if (rec_record_get_num_fields_by_name (record, fname) > 1)
            {
              ADD_ERROR (errors,
                         _("%s:%s: error: field '%s' should be unique in this record\n"),
                         rec_record_source (record),
                         rec_record_location_str (record),
                         fname);
              res++;
            }
        }
    }

 cleanup:

  rec_fex_destroy (fex_unique);
  return res;
}

static int
rec_int_check_record_prohibit (rec_rset_t rset,
                               rec_record_t record,
                               rec_buf_t errors)
{
  rec_fex_t fex_prohibit = NULL;
  int res = 0;
  size_t i;

  rec_record_t descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      fex_prohibit = rec_int_collect_field_list (descriptor, FNAME(REC_FIELD_PROHIBIT));
      if (!fex_prohibit)
        {
          ADD_ERROR (errors, _("out of memory\n"), "");
          res = 1;
          goto cleanup;
        }

      /* Make sure that no field in the prohibit fields list is
         present in the record.  */

      for (i = 0; i < rec_fex_size (fex_prohibit); i++)
        {
          const char *fname = rec_fex_elem_field_name (rec_fex_get (fex_prohibit, i));
          if (rec_record_get_num_fields_by_name (record, fname) > 0)
            {
              ADD_ERROR (errors,
                         _("%s:%s: error: prohibited field '%s' found in record\n"),
                         rec_record_source (record),
                         rec_record_location_str (record),
                         fname);
              res++;
            }
        }
    }

 cleanup:

  rec_fex_destroy (fex_prohibit);
  return res;
}

static int
rec_int_check_record_sex_constraints (rec_rset_t rset,
                                      rec_record_t record,
                                      rec_buf_t errors)
{
  int res = 0;
  size_t i = 0;
  size_t num_constraints = rec_rset_num_sex_constraints (rset);

  for (i = 0; i < num_constraints; i++)
    {
      bool status = false;
      rec_sex_t sex = rec_rset_sex_constraint (rset, i);

      if (!rec_sex_eval (sex, record, &status))
        {
          ADD_ERROR (errors,
                     _("%s:%s: error: %%constraint[%d] violated in record\n"),
                     rec_record_source (record),
                     rec_record_location_str (record),
                     i);
          res++;
        }
    }

  return res;
}

#if defined REC_CRYPT_SUPPORT

static int
rec_int_check_record_secrets (rec_rset_t rset,
                              rec_record_t record,
                              rec_buf_t errors)
{
  int res;
  rec_field_t field;
  rec_mset_iterator_t iter;

  res = 0;

  iter = rec_mset_iterator (rec_record_mset (record));
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void**) &field, NULL))
    {
      /* If the field is confidential it must be encrypted.  Encrypted
         field values can be recognized by the "encrypted-"
         prefix.  */
#define REC_ENCRYPTED_PREFIX "encrypted-"
      if (rec_rset_field_confidential_p (rset, rec_field_name (field))
          && (strncmp (rec_field_value (field),
                       REC_ENCRYPTED_PREFIX,
                       strlen (REC_ENCRYPTED_PREFIX)) != 0))
        {
          ADD_ERROR (errors,
                    _("%s:%s: error: confidential field is not encrypted\n"),
                    rec_record_source (record),
                     rec_record_location_str (record));
          res++;
        }
    }

  rec_mset_iterator_free (&iter);

  return res;
}

#endif /* REC_CRYPT_SUPPORT */

static int
rec_int_check_record_key (rec_rset_t rset,
                          rec_record_t orig_record,
                          rec_record_t record,
                          rec_buf_t errors)
{
  int res;
  rec_record_t descriptor;
  rec_record_t other_record;
  rec_mset_iterator_t iter;
  char *key_field_name;
  rec_field_t field;
  rec_field_t key;
  rec_field_t other_key;
  bool duplicated_key;
  size_t i;
  size_t num_fields;
  
  res = 0;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      for (i = 0; i < rec_record_get_num_fields_by_name (descriptor,
                                                         FNAME(REC_FIELD_KEY));
           i++)
        {
          field = rec_record_get_field_by_name (descriptor, FNAME(REC_FIELD_KEY), i);

          /* Parse the field name from the value of %key:  */
          key_field_name = rec_parse_field_name_str (rec_field_value (field));
          if (key_field_name)
            {
              num_fields = rec_record_get_num_fields_by_name (record, key_field_name);

              if (num_fields == 0)
                {
                  ADD_ERROR (errors,
                            _("%s:%s: error: key field '%s' not found in record\n"),
                             rec_record_source (record),
                             rec_record_location_str (record),
                             rec_field_value (field));
                  res++;
                }
              else if (num_fields > 1)
                {
                  ADD_ERROR (errors,
                             _("%s:%s: error: multiple key fields '%s' in record\n"),
                             rec_record_source (record),
                             rec_record_location_str (record),
                             rec_field_value (field));
                  res++;
                }
              else  /* num_fields == 1 */
                {
                  /* Check that the value specified as the key is
                     unique in the whole record set.  */
                  key = rec_record_get_field_by_name (record,
                                                      key_field_name,
                                                      0);
                  duplicated_key = false;
                  
                  iter = rec_mset_iterator (rec_rset_mset (rset));
                  while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void**) &other_record, NULL))
                    {
                      if (other_record != orig_record)
                        {
                          /* XXX: Only the first key field is considered.  */
                          other_key = rec_record_get_field_by_name (other_record,
                                                                    key_field_name,
                                                                    0);
                          if (other_key)
                            {
                              if (strcmp (rec_field_value (other_key),
                                          rec_field_value (key)) == 0)
                                {
                                  /* Found a key field with the same
                                     value in other record.  */
                                  duplicated_key = true;
                                  break;
                                }
                            }
                        }
                    }

                  rec_mset_iterator_free (&iter);

                  if (duplicated_key)
                    {
                      ADD_ERROR (errors,
                                 _("%s:%s: error: duplicated key value in field '%s' in record\n"),
                                 rec_record_source (orig_record),
                                 rec_record_location_str (orig_record),
                                 rec_field_name (key));
                      res++;
                      break;
                    }
                }

              free (key_field_name);
            }
        }                                          
    }

  return res;
}

static int
rec_int_check_descriptor (rec_rset_t rset,
                          rec_buf_t errors)
{
  int res;
  rec_record_t descriptor;
  rec_mset_iterator_t iter;
  rec_field_t field;
  const char *field_name;
  const char *field_value;
  rec_fex_t fex;
  const char *auto_field_name;
  size_t i;
  rec_type_t type;
  char *type_name = NULL;
  const char *p, *q = NULL;

  res = 0;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      /* Check the type of the record set:

         1. There should be one (and only one) %rec: field in the
            record.
         2. The value of the %rec: field shall be well-formed.
      */
      if (rec_record_get_num_fields_by_name (descriptor, FNAME(REC_FIELD_REC)) == 0)
        {
          ADD_ERROR (errors,
                     _("%s:%s: error: missing %%rec field in record descriptor\n"),
                     rec_record_source (descriptor),
                     rec_record_location_str (descriptor));
          res++;
        }
      else if (rec_record_get_num_fields_by_name (descriptor, FNAME(REC_FIELD_REC)) > 1)
        {
          ADD_ERROR (errors,
                     _("%s:%s: error: too many %%rec fields in record descriptor\n"),
                     rec_record_source (descriptor),
                     rec_record_location_str (descriptor));
          res++;
        }

      field = rec_record_get_field_by_name (descriptor, FNAME(REC_FIELD_REC), 0);
      if (!rec_int_rec_type_p (rec_field_value (field)))
        {
          ADD_ERROR (errors,
                     _("%s:%s: error: invalid record type %s\n"),
                     rec_field_source (field),
                     rec_field_location_str (field),
                     rec_field_value (field));
          res++;
        }

      /* Only one 'key:' entry is allowed, if any.  */
      if (rec_record_get_num_fields_by_name (descriptor, FNAME(REC_FIELD_KEY)) > 1)
        {
          ADD_ERROR (errors,
                     _("%s:%s: error: only one %%key field is allowed in a record descriptor\n"),
                     rec_record_source (descriptor),
                     rec_record_location_str (descriptor));
          res++;
        }

      /* Only one 'size:' entry is allowed, if any.  */
      if (rec_record_get_num_fields_by_name (descriptor, FNAME(REC_FIELD_SIZE)) > 1)
        {
          ADD_ERROR (errors,
                    _("%s:%s: error: only one %%size field is allowed in a record descriptor\n"),
                    rec_record_source (descriptor),
                    rec_record_location_str (descriptor));
          res++;
        }

      /* Only one 'sort:' entry is allowed, if any.  */
      if (rec_record_get_num_fields_by_name (descriptor, FNAME(REC_FIELD_SORT)) > 1)
        {
          ADD_ERROR (errors,
                    _("%s:%s: error: only one %%sort field is allowed in a record descriptor\n"),
                    rec_record_source (descriptor),
                    rec_record_location_str (descriptor));
          res++;
        }

      /* Iterate on fields.  */

      iter = rec_mset_iterator (rec_record_mset (descriptor));
      while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void**) &field, NULL))
        {
          field_name = rec_field_name (field);
          field_value = rec_field_value (field);

          if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_TYPE)))
            {
              /* Check for the list of fields.  */
              p = field_value;
              rec_skip_blanks (&p);
              if (!rec_parse_regexp (&p, "^" REC_FNAME_RE "(," REC_FNAME_RE ")*",
                                     NULL))
                {
                  ADD_ERROR (errors,
                            _("%s:%s: error: expected a comma-separated list of fields \
before the type specification\n"),
                            rec_field_source (field),
                            rec_field_location_str (field));
                  res++;
                }

              /* Check the type descriptor.  Note that it can be
                 either a type specification or a type name.  */
              rec_skip_blanks (&p);
              if (!rec_type_descr_p (p))
                {
                  q = p;
                  if (rec_parse_regexp (&q, "^" REC_TYPE_NAME_RE "[ \t\n]*$",
                                        NULL))
                    {
                      /* The named type shall exist in the record set
                         type registry.
                      
                         XXX: but this is probably a warning rather
                         than an error.  */

                      rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE, &type_name);
                      if (!rec_type_reg_get (rec_rset_get_type_reg (rset), type_name))
                        {
                          ADD_ERROR (errors,
                                    _("%s:%s: error: the referred type %s \
does not exist\n"),
                                    rec_field_source (field),
                                    rec_field_location_str (field),
                                    type_name);
                          res++;
                        }
                    }
                  else
                    {
                      /* XXX: make rec_type_descr_p to report more details.  */
                      ADD_ERROR (errors,
                                _("%s:%s: error: invalid type specification\n"),
                                rec_field_source (field),
                                rec_field_location_str (field));
                      res++;
                    }
                }
            }
          else if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_TYPEDEF)))
            {
              /* Check for the type name.  */
              p = field_value;
              rec_skip_blanks (&p);
              if (!rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE, NULL))
                {
                  ADD_ERROR (errors,
                            _("%s:%s: error: expected a type name before the type \
specification\n"),
                            rec_field_source (field),
                            rec_field_location_str (field));
                  res++;
                }
              
              /* Check the type descriptor.  Note that it can be
                 either a type specification or a type name.  */
              rec_skip_blanks (&p);
              if (!rec_type_descr_p (p))
                {
                  q = p;
                  if (rec_parse_regexp (&q, "^" REC_TYPE_NAME_RE "[ \t\n]*$",
                                        NULL))
                    {
                      /* The named type shall exist in the record set
                         type registry.
                      
                         XXX: but this is probably a warning rather
                         than an error.  */

                      rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE, &type_name);
                      if (!rec_type_reg_get (rec_rset_get_type_reg (rset), type_name))
                        {
                          ADD_ERROR (errors,
                                    _("%s:%s: error: the referred type %s \
does not exist\n"),
                                    rec_field_source (field),
                                    rec_field_location_str (field),
                                    type_name);
                          res++;
                        }
                    }
                  else
                    {
                      /* XXX: make rec_type_descr_p to report more details.  */
                      ADD_ERROR (errors,
                                _("%s:%s: error: invalid typedef specification\n"),
                                rec_field_source (field),
                                rec_field_location_str (field));
                      res++;
                    }
                }
            }
          else if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_CONSTRAINT)))
            {
              /* Check that the value of this field is a valid
                 selection expression.  */
              
              rec_sex_t sex = rec_sex_new (false);
              if (sex)
                {
                  if (rec_sex_compile (sex, field_value))
                    {
                      rec_sex_destroy (sex);
                    }
                  else
                    {
                      ADD_ERROR (errors,
                                 _("%s:%s: error: value for %s[%zd] is not a valid selection expression\n"),
                                 rec_record_source (descriptor),
                                 rec_record_location_str (descriptor),
                                 rec_field_name (field),
                                 rec_record_get_field_index_by_name (descriptor, field));
                      res++;
                    }
                }
              else
                {
                  /* Out of memory.  */
                  res++;
                }
            }
          else if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_MANDATORY))
                   || rec_field_name_equal_p (field_name, FNAME(REC_FIELD_UNIQUE))
                   || rec_field_name_equal_p (field_name, FNAME(REC_FIELD_PROHIBIT))
                   || rec_field_name_equal_p (field_name, FNAME(REC_FIELD_AUTO))
                   || rec_field_name_equal_p (field_name, FNAME(REC_FIELD_SORT))
                   || rec_field_name_equal_p (field_name, FNAME(REC_FIELD_ALLOWED)))
            {
              /* Check that the value of this field is a parseable
                 list of field names.  */
              fex = rec_fex_new (field_value, REC_FEX_SIMPLE);
              if (fex)
                {
                  rec_fex_destroy (fex);
                }
              else
                {
                  ADD_ERROR (errors,
                             _("%s:%s: error: value for %s[%zd] is not a list of field names\n"),
                             rec_record_source (descriptor),
                             rec_record_location_str (descriptor),
                             rec_field_name (field),
                             rec_record_get_field_index_by_name (descriptor, field));
                  res++;
                }
            }
          else if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_SIZE)))
            {
              if (!rec_match (field_value, REC_INT_SIZE_RE))
                {
                  ADD_ERROR (errors,
                            _("%s:%s: error: value for %s should be a number optionally preceded by >, <, >= or <=.\n"),
                            rec_field_source (field),
                            rec_field_location_str (field),
                            field_name);
                  res++;
                }
            }
#if defined REC_CRYPT_SUPPORT
          else if (rec_field_name_equal_p (field_name, FNAME(REC_FIELD_CONFIDENTIAL)))
            {
              if (!rec_match (field_value,
                              "^"
                              "[ \n\t]*" REC_FNAME_RE "([ \n\t]+" REC_FNAME_RE ")*"
                              "[ \n\t]*$"))
                {
                  ADD_ERROR (errors,
                            _("%s:%s: error: value for %s should be a list of field names.\n"),
                            rec_field_source (field),
                            rec_field_location_str (field),
                            field_name);
                  res++;
                }
            }
#endif /* REC_CRYPT_SUPPORT */          

          if ((rec_field_name_equal_p (field_name, FNAME(REC_FIELD_AUTO)))
              && (fex = rec_fex_new (field_value, REC_FEX_SIMPLE)))
            {
              /* Check that the auto incremented fields have not been
                 declared with a type other than 'int'. */
              for (i = 0; i < rec_fex_size (fex); i++)
                {
                  auto_field_name = rec_fex_elem_field_name (rec_fex_get (fex, i));
                  type = rec_rset_get_field_type (rset, auto_field_name);
                  if ((!type) ||
                      ! ((rec_type_kind (type) == REC_TYPE_INT)
                         || (rec_type_kind (type) == REC_TYPE_RANGE)
#if defined UUID_TYPE
                         || (rec_type_kind (type) == REC_TYPE_UUID)
#endif
                         || (rec_type_kind (type) == REC_TYPE_DATE)))
                    {
                      ADD_ERROR (errors,
#if defined UUID_TYPE
                                 _("%s:%s: error: auto-incremented field %s should be of type int, range, uuid or date\n"),
#else
                                 _("%s:%s: error: auto-incremented field %s should be of type int, range or date\n"),
#endif
                                rec_record_source (descriptor),
                                rec_record_location_str (descriptor),
                                auto_field_name);
                      res++;
                    }
                }
            }
        }

      rec_mset_iterator_free (&iter);
    }

  return res;
}

int
rec_int_merge_remote (rec_rset_t rset,
                      rec_buf_t errors)
{
  int res;
  rec_parser_t parser;
  rec_record_t descriptor;
  rec_db_t remote_db;
  rec_rset_t remote_rset;
  rec_field_t remote_field;
  rec_mset_iterator_t iter;
  rec_record_t remote_descriptor;
  rec_field_t rec_field;
  char *rec_type;
  char *rec_url = NULL;
  char *rec_file = NULL;
  char *rec_source = NULL;
  FILE *external_file;
  char tmpfile_name[14];

  res = 0;

  tmpfile_name[0] = '\0';

  /* If a remote descriptor is defined in the record descriptor of
     RSET, fetch it and merge it with the local descriptor.  */

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      /* Check if there is an URL in the %rec: field.  */
      rec_field = rec_record_get_field_by_name (descriptor, FNAME(REC_FIELD_REC), 0);

      if (!rec_int_rec_type_p (rec_field_value (rec_field)))
        {
          return 0;
        }

      rec_type = rec_extract_type (rec_field_value (rec_field));
      rec_file = rec_extract_file (rec_field_value (rec_field));
      rec_url  = rec_extract_url  (rec_field_value (rec_field));

      if (rec_file || rec_url)
        {
          if (rec_url)
            {
#if defined REMOTE_DESCRIPTORS
              CURL *curl;
              int tmpfile_des;

              /* Fetch the remote descriptor.  */
              curl = curl_easy_init ();

              /* Create a temporary file.  */
              memcpy (tmpfile_name, "recint-XXXXXX", 13);
              tmpfile_name[13] = '\0';
              tmpfile_des = gen_tempname (tmpfile_name, 0, 0, GT_FILE);
              external_file = fdopen (tmpfile_des, "r+");
              
              /* Fetch the remote file.  */
              curl_easy_setopt (curl, CURLOPT_URL, rec_url);
              curl_easy_setopt (curl, CURLOPT_WRITEDATA, external_file);
              curl_easy_setopt (curl, CURLOPT_FAILONERROR, 1);
              if (curl_easy_perform (curl) != 0)
                {
                  ADD_ERROR (errors,
                             _("%s:%s: error: could not fetch remote descriptor from url %s.\n"),
                             rec_field_source (rec_field), rec_field_location_str (rec_field),
                             rec_url);
                  res++;
                  goto exit;
                }
              curl_easy_cleanup (curl);
              rec_source = rec_url;
#else
              goto exit;
#endif /* REMOTE_DESCRIPTORS */
            }
          else
            {
              /* Try to open the file.  */
              external_file = fopen (rec_file, "r");
              if (!external_file)
                {
                  ADD_ERROR (errors,
                             _("%s:%s: error: could not read external descriptor from file %s.\n"),
                             rec_field_source (rec_field), rec_field_location_str (rec_field),
                             rec_file);
                  res++;
                  goto exit;
                }
              rec_source = rec_file;
            }              

          /* Parse the contents of the external file.  */
          fseek (external_file, 0, SEEK_SET);
          parser = rec_parser_new (external_file, rec_source);
          if (!rec_parse_db (parser, &remote_db))
            {
              ADD_ERROR (errors,
                         _("%s:%s: error: %s does not contain valid rec data.\n"),
                         rec_field_source (rec_field), rec_field_location_str (rec_field),
                         rec_source);
              res++;
              goto exit;
            }
          rec_parser_destroy (parser);
          
          /* Get the proper external descriptor and merge it with
             the local one.  */
          remote_rset = rec_db_get_rset_by_type (remote_db, rec_type);
          if (!remote_rset)
            {
              ADD_ERROR (errors,
                         _("%s:%s: error: %s does not contain information for type %s.\n"),
                         rec_field_source (rec_field), rec_field_location_str (rec_field),
                         rec_source, rec_type);
              res++;
              goto exit;
            }
          remote_descriptor = rec_rset_descriptor (remote_rset);
          if (!remote_descriptor)
            {
              /* Do nothing.  */
              goto exit;
            }
          
          iter = rec_mset_iterator (rec_record_mset (remote_descriptor));
          while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void**) &remote_field, NULL))
            {
              /* Merge the descriptors, but take care to not add a new
                 %rec: field.  */

              if (!rec_field_name_equal_p (rec_field_name (remote_field), FNAME(REC_FIELD_REC)))
                {
                  rec_mset_append (rec_record_mset (descriptor), MSET_FIELD, (void *) rec_field_dup (remote_field), MSET_ANY);
                }
            }

          rec_mset_iterator_free (&iter);
          
          /* Update the record descriptor (triggering the creation
             of a new type registry).  */
          rec_rset_set_descriptor (rset, rec_record_dup (descriptor));
          
          rec_db_destroy (remote_db);
          fclose (external_file);
        }
    }

 exit:

  if (rec_url && (tmpfile_name[0] != '\0'))
    {
      remove (tmpfile_name);
    }

  free (rec_url);
  free (rec_file);

  return res;
}

static bool
rec_int_rec_type_p (const char *str)
{
  return rec_match (str,
                    "^[ \t]*"
                    REC_RECORD_TYPE_RE
                    "[ \n\t]*"
                    "("
                    "(" REC_URL_REGEXP ")"
                    "|"
                    "(" REC_FILE_REGEXP ")"
                    "[ \t]*)?"
                    "$");
}

/* End of rec-int.c */
