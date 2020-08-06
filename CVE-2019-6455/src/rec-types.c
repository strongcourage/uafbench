/* -*- mode: C -*-
 *
 *       File:         rec-types.c
 *       Date:         Fri Apr 23 14:10:05 2010
 *
 *       GNU recutils - Field types.
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

#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <limits.h>
#include <regex.h>
#include <parse-datetime.h>
#include <gettext.h>
#define _(str) dgettext (PACKAGE, str)

#if defined UUID_TYPE
#  include <uuid/uuid.h>
#endif

#include <rec-utils.h>
#include <rec.h>

/*
 * Constants.
 */

/* Textual name of the types expected in the type description
   strings.  */

#define REC_TYPE_INT_NAME    "int"
#define REC_TYPE_BOOL_NAME   "bool"
#define REC_TYPE_RANGE_NAME  "range"
#define REC_TYPE_REAL_NAME   "real"
#define REC_TYPE_SIZE_NAME   "size"
#define REC_TYPE_LINE_NAME   "line"
#define REC_TYPE_REGEXP_NAME "regexp"
#define REC_TYPE_DATE_NAME   "date"
#define REC_TYPE_ENUM_NAME   "enum"
#define REC_TYPE_EMAIL_NAME  "email"
#define REC_TYPE_FIELD_NAME  "field"
#define REC_TYPE_REC_NAME    "rec"

#if defined UUID_TYPE
#  define REC_TYPE_UUID_NAME   "uuid"
#endif

/* Regular expressions.  */

#define REC_TYPE_BLANK_RE "[ \t\n]"
#define REC_TYPE_NO_BLANK_RE "[^ \t\n]"
#define REC_TYPE_BLANKS_RE REC_TYPE_BLANK_RE "+"
#define REC_TYPE_NO_BLANKS_RE REC_TYPE_NO_BLANK_RE "+"
#define REC_TYPE_ZBLANKS_RE REC_TYPE_BLANK_RE "*"

/* Regular expressions denoting values.  */
#define REC_TYPE_INT_VALUE_RE                   \
  "^" REC_TYPE_ZBLANKS_RE REC_INT_RE REC_TYPE_ZBLANKS_RE "$"

#define REC_TYPE_BOOL_TRUE_VALUES_RE  "1|yes|true"
#define REC_TYPE_BOOL_FALSE_VALUES_RE "0|no|false"

#define REC_TYPE_BOOL_VALUE_RE                  \
  "^" REC_TYPE_ZBLANKS_RE "(" REC_TYPE_BOOL_TRUE_VALUES_RE "|" \
                              REC_TYPE_BOOL_FALSE_VALUES_RE    \
                          ")" REC_TYPE_ZBLANKS_RE "$"

#define REC_TYPE_REAL_VALUE_RE                  \
  "^" REC_TYPE_ZBLANKS_RE "-?([0-9]+)?(\\.[0-9]+)?" REC_TYPE_ZBLANKS_RE "$"

#define REC_TYPE_LINE_VALUE_RE                  \
  "^[^\n]*$"

#define REC_TYPE_EMAIL_VALUE_RE                 \
  "^[ \n\t]*"                                   \
  "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}" \
  "[ \n\t]*$"

#define REC_TYPE_ENUM_NAME_RE                   \
  "[a-zA-Z0-9][a-zA-Z0-9_-]*"
#define REC_TYPE_ENUM_VALUE_RE                  \
  "^"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  REC_TYPE_ENUM_NAME_RE                         \
  REC_TYPE_ZBLANKS_RE                           \
  "$"

/* REC_FNAME_RE is defined in rec.h */
#define REC_TYPE_FIELD_VALUE_RE                 \
  "^"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  REC_FNAME_RE                                  \
  REC_TYPE_ZBLANKS_RE                           \
  "$"
  
#define REC_TYPE_REC_VALUE_RE

/* Regular expression denoting a type name.  */

#if defined UUID_TYPE
#  define REC_TYPE_CLASS_UUID_RE "|" REC_TYPE_UUID_NAME
#else
#  define REC_TYPE_CLASS_UUID_RE
#endif

#define REC_TYPE_CLASS_RE                               \
  "(" REC_TYPE_INT_NAME   "|" REC_TYPE_RANGE_NAME  "|" \
      REC_TYPE_REAL_NAME  "|" REC_TYPE_SIZE_NAME   "|" \
      REC_TYPE_LINE_NAME  "|" REC_TYPE_REGEXP_NAME "|" \
      REC_TYPE_DATE_NAME  "|" REC_TYPE_ENUM_NAME   "|" \
      REC_TYPE_EMAIL_NAME "|" REC_TYPE_BOOL_NAME   "|" \
      REC_TYPE_FIELD_NAME "|" REC_TYPE_REC_NAME        \
      REC_TYPE_CLASS_UUID_RE                           \
  ")"

/* Regular expressions for the type descriptions.  */

/* int  */
#define REC_TYPE_INT_DESCR_RE                      \
  REC_TYPE_INT_NAME

/* bool */
#define REC_TYPE_BOOL_DESCR_RE                  \
  REC_TYPE_BOOL_NAME

/* range MIN MAX  */
#define REC_TYPE_RANGE_MINMAX_RE "MIN | MAX"
#define REC_TYPE_RANGE_DESCR_RE                    \
  REC_TYPE_RANGE_NAME                              \
  REC_TYPE_BLANKS_RE                               \
  "(" REC_INT_RE "|" REC_TYPE_RANGE_MINMAX_RE ")"  \
  "("                                              \
     REC_TYPE_ZBLANKS_RE                           \
     "(" REC_INT_RE "|" REC_TYPE_RANGE_MINMAX_RE ")"\
  ")?"

/* real  */
#define REC_TYPE_REAL_DESCR_RE                     \
  REC_TYPE_REAL_NAME

/* size NUM  */
#define REC_TYPE_SIZE_DESCR_RE                     \
  REC_TYPE_SIZE_NAME                               \
  REC_TYPE_BLANKS_RE                               \
  REC_INT_RE

/* line  */
#define REC_TYPE_LINE_DESCR_RE                  \
  REC_TYPE_LINE_NAME

/* regexp /RE/  */
#define REC_TYPE_REGEXP_DESCR_RE                 \
  REC_TYPE_REGEXP_NAME                           \
  ".+"

/* date  */
#define REC_TYPE_DATE_DESCR_RE                  \
  REC_TYPE_DATE_NAME

/* enum NAME NAME NAME*/
#define REC_TYPE_ENUM_DESCR_RE                  \
  REC_TYPE_ENUM_NAME                            \
  REC_TYPE_BLANKS_RE                            \
  REC_TYPE_ENUM_NAME_RE                         \
  "(" REC_TYPE_BLANKS_RE REC_TYPE_ENUM_NAME_RE ")*"

/* field */
#define REC_TYPE_FIELD_DESCR_RE                 \
  REC_TYPE_FIELD_NAME

/* rec RECORD_TYPE */
#define REC_TYPE_REC_DESCR_RE                   \
  REC_TYPE_REC_NAME REC_TYPE_BLANKS_RE REC_RECORD_TYPE_RE

/* email */
#define REC_TYPE_EMAIL_DESCR_RE                 \
  REC_TYPE_EMAIL_NAME

/* Regexp denoting any type description.  */
#define REC_TYPE_DESCR_RE                       \
  "^"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  REC_FNAME_RE "(," REC_FNAME_RE ")*"           \
  REC_TYPE_ZBLANKS_RE                           \
  "("                                           \
         "(" REC_TYPE_INT_DESCR_RE    ")"       \
     "|" "(" REC_TYPE_BOOL_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_RANGE_DESCR_RE  ")"       \
     "|" "(" REC_TYPE_REAL_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_SIZE_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_LINE_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_REGEXP_DESCR_RE ")"       \
     "|" "(" REC_TYPE_DATE_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_EMAIL_DESCR_RE  ")"       \
     "|" "(" REC_TYPE_ENUM_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_FIELD_DESCR_RE  ")"       \
     "|" "(" REC_TYPE_REC_DESCR_RE    ")"       \
  ")"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  "$"
      
/*
 * Data types.
 */

struct rec_type_s
{
  char *name;                 /* Name of the type.  May be NULL in an
                                 anonymous type.  */
  enum rec_type_kind_e kind;  /* Kind of the type.  */
  char *expr;                 /* Copy of the type descriptor used to
                                 create the type.  */
  size_t size;                /* Used for enumerations: number of
                                 names.  */

  union
  {
    size_t max_size;          /* Size of string.  */
    int range[2];             /* Range.  */
    regex_t regexp;           /* Regular expression.  */
    char *recname;            /* Record.  */

#define REC_ENUM_ALLOC_NAMES 50
    char **names;             /* Names in enumeration.  */
  } data;
};

#define REC_TYPE_REG_ALLOC_TYPES 100

struct rec_type_reg_entry_s
{
  char *type_name;

  rec_type_t type;
  char *to_type;
  bool visited_p;
};

struct rec_type_reg_s
{
  size_t num_types;
  struct rec_type_reg_entry_s *types;
};

/*
 * Forward declarations.
 */

static enum rec_type_kind_e rec_type_parse_type_kind (char *str);

static bool rec_type_check_int (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_bool (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_range (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_real (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_size (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_line (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_regexp (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_date (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_email (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_enum (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_field (rec_type_t type, const char *str, rec_buf_t errors);
static bool rec_type_check_rec (rec_type_t type, const char *str, rec_buf_t errors);

#if defined UUID_TYPE
static bool rec_type_check_uuid (rec_type_t type, const char *str, rec_buf_t errors);
#endif

/* Parsing routines.  */

static const char *rec_type_parse_size (const char *str, rec_type_t type);
static const char *rec_type_parse_enum (const char *str, rec_type_t type);
static const char *rec_type_parse_regexp_type (const char *str, rec_type_t type);
static const char *rec_type_parse_range (const char *str, rec_type_t type);
static const char *rec_type_parse_rec (const char *str, rec_type_t type);

/*
 * Public functions.
 */

bool
rec_type_descr_p (const char *str)
{
  bool ret;
  rec_type_t aux_type;

  ret = false;
  
  aux_type = rec_type_new (str);
  if (aux_type)
    {
      ret = true;
      rec_type_destroy (aux_type);
    }

  return ret;
}

char *
rec_type_descr_type (char *str)
{
  char *result = NULL;
  char *name;
  const char *p;

  if (rec_type_descr_p (str))
    {
      p = str;

      /* Skip blank characters.  */
      rec_skip_blanks (&p);

      /* Skip the FEX  */
      if (rec_parse_regexp (&p, "^" REC_FNAME_RE "(," REC_FNAME_RE ")*",
                            &name))
        {
          free (name);
        }

      /* Skip blanks.  */
      rec_skip_blanks (&p);

      /* Return the rest of the string.  */
      result = strdup (p);
    }

  return result;
}

rec_type_t
rec_type_new (const char *str)
{
  rec_type_t new;
  const char *p;
  char *type_kind_str = NULL;

  p = str;
  new = malloc (sizeof (struct rec_type_s));
  if (!new)
    {
      goto exit;
    }
  new->name = NULL; /* Newly created types are anonyous. */
  new->size = 0;

  rec_skip_blanks (&p);

  /* Get the type kind.  */
  if (!rec_parse_regexp (&p, "^" REC_TYPE_CLASS_RE, &type_kind_str))
    {
      free (new);
      new = NULL;
      goto exit;
    }

  /* Continue parsing depending on the kind of type.  */
  new->kind = rec_type_parse_type_kind (type_kind_str);
  switch (new->kind)
    {
    case REC_TYPE_SIZE:
      {
        p = rec_type_parse_size (p, new);
        if (!p)
          {
            free (new);
            new = NULL;
          }        
        break;
      }
    case REC_TYPE_ENUM:
      {
        p = rec_type_parse_enum (p, new);
        if (!p)
          {
            free (new);
            new = NULL;
          }
        break;
      }
    case REC_TYPE_REGEXP:
      {
        p = rec_type_parse_regexp_type (p, new);
        if (!p)
          {
            free (new);
            new = NULL;
          }
        break;
      }
    case REC_TYPE_RANGE:
      {
        p = rec_type_parse_range (p, new);
        if (!p)
          {
            free (new);
            new = NULL;
          }
        break;
      }
    case REC_TYPE_REC:
      {
        p = rec_type_parse_rec (p, new);
        if (!p)
          {
            free (new);
            new = NULL;
          }
        break;
      }
    case REC_TYPE_INT:
    case REC_TYPE_BOOL:
    case REC_TYPE_REAL:
    case REC_TYPE_LINE:
    case REC_TYPE_FIELD:
    case REC_TYPE_DATE:
    case REC_TYPE_EMAIL:
#if defined UUID_TYPE
    case REC_TYPE_UUID:
#endif
      {
        /* We are done.  */
        break;
      }
    case REC_TYPE_NONE:
      {
        /* This point should not be reached.  */
        fprintf (stderr,
                 _("internal error: rec-types: got REC_TYPE_NONE from rec_type_parse_type_kind() in rec_type_new().\n"));
        exit (EXIT_FAILURE);
        break;
      }
    }

  if (new)
    {
      /* Check that all characters until the end of the string are
         blank characters.  */
      while (*p != '\0')
        {
          if (!rec_blank_p (*p))
            {
              rec_type_destroy (new);
              new = NULL;
              break;
            }
          
          p++;
        }
    }

 exit:

  free (type_kind_str);

  return new;
}

enum rec_type_kind_e
rec_type_kind (rec_type_t type)
{
  return type->kind;
}

char *
rec_type_kind_str (rec_type_t type)
{
  char *res;

  switch (type->kind)
    {
    case REC_TYPE_NONE:
      {
        res = "";
        break;
      }
    case REC_TYPE_INT:
      {
        res = REC_TYPE_INT_NAME;
        break;
      }
    case REC_TYPE_BOOL:
      {
        res = REC_TYPE_BOOL_NAME;
        break;
      }
    case REC_TYPE_RANGE:
      {
        res = REC_TYPE_RANGE_NAME;
        break;
      }
    case REC_TYPE_REAL:
      {
        res = REC_TYPE_REAL_NAME;
        break;
      }
    case REC_TYPE_SIZE:
      {
        res = REC_TYPE_SIZE_NAME;
        break;
      }
    case REC_TYPE_LINE:
      {
        res = REC_TYPE_LINE_NAME;
        break;
      }
    case REC_TYPE_REGEXP:
      {
        res = REC_TYPE_REGEXP_NAME;
        break;
      }
    case REC_TYPE_DATE:
      {
        res = REC_TYPE_DATE_NAME;
        break;
      }
    case REC_TYPE_EMAIL:
      {
        res = REC_TYPE_EMAIL_NAME;
        break;
      }
    case REC_TYPE_ENUM:
      {
        res = REC_TYPE_ENUM_NAME;
        break;
      }
    case REC_TYPE_FIELD:
      {
        res = REC_TYPE_FIELD_NAME;
        break;
      }
    case REC_TYPE_REC:
      {
        res = REC_TYPE_REC_NAME;
        break;
      }
#if defined UUID_TYPE
    case REC_TYPE_UUID:
      {
        res = REC_TYPE_UUID_NAME;
        break;
      }
#endif
    default:
      {
        res = REC_TYPE_NONE;
        break;
      }
    }

  return res;
}

bool
rec_type_check (rec_type_t type,
                const char *str,
                char **error_str)
{
  bool res;
  rec_buf_t errors;
  char *err_str;
  size_t errors_size;

  errors = rec_buf_new (&err_str, &errors_size);

  res = false;
  switch (type->kind)
    {
    case REC_TYPE_NONE:
      {
        res = true;
        break;
      }
    case REC_TYPE_INT:
      {
        res = rec_type_check_int (type, str, errors);
        break;
      }
    case REC_TYPE_BOOL:
      {
        res = rec_type_check_bool (type, str, errors);
        break;
      }
    case REC_TYPE_RANGE:
      {
        res = rec_type_check_range (type, str, errors);
        break;
      }
    case REC_TYPE_REAL:
      {
        res = rec_type_check_real (type, str, errors);
        break;
      }
    case REC_TYPE_SIZE:
      {
        res = rec_type_check_size (type, str, errors);
        break;
      }
    case REC_TYPE_LINE:
      {
        res = rec_type_check_line (type, str, errors);
        break;
      }
    case REC_TYPE_REGEXP:
      {
        res = rec_type_check_regexp (type, str, errors);
        break;
      }
    case REC_TYPE_DATE:
      {
        res = rec_type_check_date (type, str, errors);
        break;
      }
    case REC_TYPE_EMAIL:
      {
        res = rec_type_check_email (type, str, errors);
        break;
      }
    case REC_TYPE_ENUM:
      {
        res = rec_type_check_enum (type, str, errors);
        break;
      }
    case REC_TYPE_FIELD:
      {
        res = rec_type_check_field (type, str, errors);
        break;
      }
    case REC_TYPE_REC:
      {
        res = rec_type_check_rec (type, str, errors);
        break;
      }
#if defined UUID_TYPE
    case REC_TYPE_UUID:
      {
        res = rec_type_check_uuid (type, str, errors);
        break;
      }
#endif
    }

  /* Terminate the 'errors' string.  */
  rec_buf_close (errors);
  /*  err_str[errors_size] = '\0';*/

  if (error_str)
    {
      *error_str = err_str;
    }
  else
    {
      free (err_str);
    }

  return res;
}

void
rec_type_destroy (rec_type_t type)
{
  int i;

  if (type)
    {
      if (type->kind == REC_TYPE_ENUM)
        {
          for (i = 0; i < type->size; i++)
            {
              free (type->data.names[i]);
            }
        }
      else if (type->kind == REC_TYPE_REGEXP)
        {
          regfree (&type->data.regexp);
        }
      
      free (type->name);
      free (type);
    }
}

rec_type_reg_t
rec_type_reg_new (void)
{
  rec_type_reg_t new;

  new = malloc (sizeof (struct rec_type_reg_s));
  if (new)
    {
      new->num_types = 0;
      new->types = NULL;
    }

  return new;
}

void
rec_type_reg_destroy (rec_type_reg_t reg)
{
  size_t i;
  
  for (i = 0; i < reg->num_types; i++)
    {
      if (reg->types[i].type)
        {
          rec_type_destroy (reg->types[i].type);
        }
      free (reg->types[i].type_name);
      free (reg->types[i].to_type);
    }
  free (reg->types);
  free (reg);
}

void
rec_type_reg_add (rec_type_reg_t reg,
                  rec_type_t type)
{
  size_t i;
  const char *type_name = NULL;

  type_name = rec_type_name (type);
  if (!type_name)
    {
      /* The registry only contains named types.  */
      return;
    }

  for (i = 0; i < reg->num_types; i++)
    {
      if (strcmp (reg->types[i].type_name, type_name) == 0)
        {
          /* Replace this entry.  */
          if (reg->types[i].type)
            {
              rec_type_destroy (reg->types[i].type);
            }
          free (reg->types[i].type_name);
          free (reg->types[i].to_type);
          break;
        }
    }

  /* If we need to add a new entry then allocate it.  */

  if (i == reg->num_types)
    {
      reg->types =
        realloc (reg->types,
                 ((i / REC_TYPE_REG_ALLOC_TYPES) + 1) * (sizeof (struct rec_type_reg_entry_s *) * REC_TYPE_REG_ALLOC_TYPES));
      reg->num_types++;
    }

  reg->types[i].type_name = strdup (rec_type_name (type));
  reg->types[i].type = type;
  reg->types[i].to_type = NULL;
  reg->types[i].visited_p = false;
}

void
rec_type_reg_add_synonym (rec_type_reg_t reg,
                          const char *type_name,
                          const char *to_type)
{
  size_t i;

  for (i = 0; i < reg->num_types; i++)
    {
      if (strcmp (reg->types[i].type_name, type_name) == 0)
        {
          /* Replace this entry.  */
          if (reg->types[i].type)
            {
              rec_type_destroy (reg->types[i].type);
            }
          free (reg->types[i].type_name);
          free (reg->types[i].to_type);
          break;
        }
    }


  /* If we need to add a new entry then allocate it.  */

  if (i == reg->num_types)
    {
      reg->types =
        realloc (reg->types,
                 ((i / REC_TYPE_REG_ALLOC_TYPES) + 1) * (sizeof (struct rec_type_reg_entry_s *) * REC_TYPE_REG_ALLOC_TYPES));
      reg->num_types++;
    }

  reg->types[i].type_name = strdup (type_name);
  reg->types[i].to_type = strdup (to_type);
  reg->types[i].type = NULL;
  reg->types[i].visited_p = false;
}

rec_type_t
rec_type_reg_get (rec_type_reg_t reg,
                  const char *type_name)
{
  size_t i;
  rec_type_t type = NULL;

  for (i = 0; i < reg->num_types; i++)
    {
      if (strcmp (reg->types[i].type_name, type_name) == 0)
        {
          if (reg->types[i].type)
            {
              /* Type found.  */
              type = reg->types[i].type;
              break;
            }
          else
            {
              /* Loop detection.  */
              if (reg->types[i].visited_p)
                {
                  break;
                }
              
              /* Mark this entry as visited and follow the name.  */
              reg->types[i].visited_p = true;
              type = rec_type_reg_get (reg, reg->types[i].to_type);
            }
        }
    }

  /* Reset the visited flags.  */
  for (i = 0; i < reg->num_types; i++)
    {
      reg->types[i].visited_p = false;
    }
  
  return type;
}

const char *
rec_type_name (rec_type_t type)
{
  return type->name;
}

void
rec_type_set_name (rec_type_t type, const char *name)
{
  type->name = strdup (name);
}

bool
rec_type_equal_p (rec_type_t type1,
                  rec_type_t type2)
{
  bool ret;
  size_t i;

  ret = true;

  if (type1->kind != type2->kind)
    {
      ret = false;
    }
  else
    {
      if (type1->kind == REC_TYPE_SIZE)
        {
          ret = (type1->data.max_size == type2->data.max_size);
        }
      else if (type1->kind == REC_TYPE_RANGE)
        {
          ret = ((type1->data.range[0] == type2->data.range[0])
                 && (type1->data.range[1] == type2->data.range[1]));
        }
      else if (type1->kind == REC_TYPE_ENUM)
        {
          for (i = 0; i < type1->size; i++)
            {
              ret = ((i < type2->size)
                     && (strcmp (type1->data.names[i],
                                 type2->data.names[i]) == 0));
            }
        }
      else if (type1->kind == REC_TYPE_REGEXP)
        {
          /* Since there is no way to determine whether two
             regex_t variables refer to equivalent regexps.  */
          ret = false;
        }
    }

  return ret;
}

int
rec_type_min (rec_type_t type)
{
  int res;

  if (type->kind != REC_TYPE_RANGE)
    {
      res = -1;
    }
  else
    {
      res = type->data.range[0];
    }

  return res;
}

int
rec_type_max (rec_type_t type)
{
  int res;

  if (type->kind != REC_TYPE_RANGE)
    {
      res = -1;
    }
  else
    {
      res = type->data.range[1];
    }

  return res;
}

const char *
rec_type_rec (rec_type_t type)
{
  const char *res = NULL;

  if (type->kind == REC_TYPE_REC)
    {
      res = type->data.recname;
    }

  return res;
}

int
rec_type_values_cmp (rec_type_t type,
                     const char *val1,
                     const char *val2)
{
  int type_comparison;
  enum rec_type_kind_e kind = REC_TYPE_NONE;

  if (type)
    {
      kind = type->kind;
    }

  switch (kind)
    {
    case REC_TYPE_INT:
    case REC_TYPE_RANGE:
      {
        int int1, int2 = 0;

        if (!rec_atoi (val1, &int1) || !rec_atoi (val2, &int2))
          {
            goto lexi;
          }

        if (int1 < int2)
          {
            type_comparison = -1;
          }
        else if (int1 > int2)
          {
            type_comparison = 1;
          }
        else
          {
            type_comparison = 0;
          }

        break;
      }
    case REC_TYPE_REAL:
      {
        double real1, real2 = 0;

        if (!rec_atod (val1, &real1) || !rec_atod (val2, &real2))
          {
            goto lexi;
          }

        if (real1 < real2)
          {
            type_comparison = -1;
          }
        else if (real1 > real2)
          {
            type_comparison = 1;
          }
        else
          {
            type_comparison = 0;
          }

        break;
      }
    case REC_TYPE_BOOL:
      {
        bool bool1, bool2 = false;

        /* Boolean fields storing 'false' come first.  */
            
        bool1 = rec_match (val1,
                           REC_TYPE_ZBLANKS_RE "(" REC_TYPE_BOOL_TRUE_VALUES_RE ")" REC_TYPE_ZBLANKS_RE);
        bool2 = rec_match (val2,
                           REC_TYPE_ZBLANKS_RE "(" REC_TYPE_BOOL_TRUE_VALUES_RE ")" REC_TYPE_ZBLANKS_RE);

        if (!bool1 && bool2)
          {
            type_comparison = -1;
          }
        else if (bool1 == bool2)
          {
            type_comparison = 0;
          }
        else
          {
            type_comparison = 1;
          }

        break;
      }
    case REC_TYPE_DATE:
      {
        struct timespec op1;
        struct timespec op2;
        struct timespec diff;

        if (parse_datetime (&op1, val1, NULL)
            && parse_datetime (&op2, val2, NULL))
          {
            if ((op1.tv_sec == op2.tv_sec)
                && (op1.tv_nsec == op2.tv_nsec))
              {
                /* op1 == op2 */
                type_comparison = 0;
              }
            else if (rec_timespec_subtract (&diff, &op1, &op2))
              {
                /* op1 < op2 */
                type_comparison = -1;
              }
            else
              {
                /* op1 > op2 */
                type_comparison = 1;
              }
          }
        else
          {
            /* Invalid date => lexicographic order.  */
            goto lexi;
          }

        break;
      }
    default:
          {
          lexi:
            /* Lexicographic order.  */
            type_comparison = strcmp (val1, val2);
            break;
          }
    }

  return type_comparison;
}

/*
 * Private functions.
 */

static enum rec_type_kind_e
rec_type_parse_type_kind (char *str)
{
  enum rec_type_kind_e res;

  res = REC_TYPE_NONE;

  if (strcmp (str, REC_TYPE_INT_NAME) == 0)
    {
      res = REC_TYPE_INT;
    }
  if (strcmp (str, REC_TYPE_BOOL_NAME) == 0)
    {
      res = REC_TYPE_BOOL;
    }
  if (strcmp (str, REC_TYPE_RANGE_NAME) == 0)
    {
      res = REC_TYPE_RANGE;
    }
  if (strcmp (str, REC_TYPE_REAL_NAME) == 0)
    {
      res = REC_TYPE_REAL;
    }
  if (strcmp (str, REC_TYPE_SIZE_NAME) == 0)
    {
      res = REC_TYPE_SIZE;
    }
  if (strcmp (str, REC_TYPE_LINE_NAME) == 0)
    {
      res = REC_TYPE_LINE;
    }
  if (strcmp (str, REC_TYPE_REGEXP_NAME) == 0)
    {
      res = REC_TYPE_REGEXP;
    }
  if (strcmp (str, REC_TYPE_DATE_NAME) == 0)
    {
      res = REC_TYPE_DATE;
    }
  if (strcmp (str, REC_TYPE_EMAIL_NAME) == 0)
    {
      res = REC_TYPE_EMAIL;
    }
  if (strcmp (str, REC_TYPE_ENUM_NAME) == 0)
    {
      res = REC_TYPE_ENUM;
    }
  if (strcmp (str, REC_TYPE_FIELD_NAME) == 0)
    {
      res = REC_TYPE_FIELD;
    }
  if (strcmp (str, REC_TYPE_REC_NAME) == 0)
    {
      res = REC_TYPE_REC;
    }
#if defined UUID_TYPE
  if (strcmp (str, REC_TYPE_UUID_NAME) == 0)
    {
      res = REC_TYPE_UUID;
    }
#endif

  return res;
}

static bool
rec_type_check_int (rec_type_t type,
                    const char *str,
                    rec_buf_t errors)
{
  bool ret;

  ret = rec_match (str, REC_TYPE_INT_VALUE_RE);
  if (!ret && errors)
    {
      rec_buf_puts (_("invalid integer."), errors);
    }

  return ret;
}

static bool
rec_type_check_rec (rec_type_t type,
                    const char *str,
                    rec_buf_t errors)
{
  /* The values of type 'rec' are of whatever type the primary key of
     the referred type is.  That check is not implemented here, but in
     rec-types.c.

     So sorry Mario, but the Princess is kept in another castle...  */
  
  return true;
}

static bool
rec_type_check_field (rec_type_t type,
                      const char *str,
                      rec_buf_t errors)
{
  bool ret;

  ret = rec_match (str, REC_TYPE_FIELD_VALUE_RE);
  if (!ret && errors)
    {
      rec_buf_puts (_("invalid 'field' value."), errors);
    }

  return ret;
}

#if defined UUID_TYPE

static bool
rec_type_check_uuid (rec_type_t type,
                     const char *str,
                     rec_buf_t errors)
{
  int ret;
  uuid_t uu;

  /* Determine whether the given string is a valid UUID by parsing it
     using the uuid_parse function provided by the libuuid
     library.  */

  ret = uuid_parse (str, uu);
  if ((ret == -1) && errors)
    {
      rec_buf_puts (_("invalid 'uuid' value."), errors);
    }

  return (ret == 0);
}

#endif /* UUID_TYPE */

static bool
rec_type_check_bool (rec_type_t type,
                     const char *str,
                     rec_buf_t errors)
{
  bool ret;

  ret = rec_match (str, REC_TYPE_BOOL_VALUE_RE);
  if (!ret && errors)
    {
      rec_buf_puts (_("invalid 'bool' value."), errors);
    }

  return ret;
}

static bool
rec_type_check_range (rec_type_t type,
                      const char *str,
                      rec_buf_t errors)
{
  bool ret;
  const char *p;
  int num;
  char *tmp;

  p = str;

  rec_skip_blanks (&p);
  if (!rec_parse_int (&p, &num))
    {
      if (errors)
        {
          rec_buf_puts (_("invalid 'range' value."), errors);
        }
      return false;
    }

  ret = ((num >= type->data.range[0])
         && (num <= type->data.range[1]));
  if (!ret && errors)
    {
      if (asprintf (&tmp, _("expected an integer between %d and %d."),
                    type->data.range[0], type->data.range[1]) != -1)
        {
          rec_buf_puts (tmp, errors);
          free (tmp);
        }
    }
  
  return ret;
}

static bool
rec_type_check_real (rec_type_t type,
                     const char *str,
                     rec_buf_t errors)
{
  bool ret;

  ret = rec_match (str, REC_TYPE_REAL_VALUE_RE);
  if (!ret && errors)
    {
      rec_buf_puts (_("invalid 'real' value."), errors);
    }

  return ret;
}

static bool
rec_type_check_size (rec_type_t type,
                     const char *str,
                     rec_buf_t errors)
{
  bool ret;
  char *tmp;

  ret = (strlen (str) <= type->data.max_size);
  if (!ret && errors)
    {
      if (asprintf (&tmp,
                    _("value too large.  Expected a size <= %zu."),
                    type->data.max_size) != -1)
        {
          rec_buf_puts (tmp, errors);
          free (tmp);
        }
    }
  
  return (strlen (str) <= type->data.max_size);
}

static bool
rec_type_check_line (rec_type_t type,
                     const char *str, 
                     rec_buf_t errors)
{
  bool ret;

  ret = rec_match (str, REC_TYPE_LINE_VALUE_RE);
  if (!ret && errors)
    {
      rec_buf_puts (_("invalid 'line' value."), errors);
    }

  return ret;
}

static bool
rec_type_check_regexp (rec_type_t type,
                       const char *str,
                       rec_buf_t errors)
{
  bool ret;
  ret = (regexec (&type->data.regexp,
                  str,
                  0,
                  NULL,
                  0) == 0);
  if (!ret && errors)
    {
      rec_buf_puts (_("value does not match the regexp."), errors);
    }

  return ret;
}

static bool
rec_type_check_date (rec_type_t type,
                     const char *str,
                     rec_buf_t errors)
{
  bool ret;
  struct timespec tm;

  if (strcmp (str, "") == 0)
    {
      /* The parse_datetime call accepts the empty string.  */
      return false;
    }

  ret = parse_datetime (&tm, str, NULL);
  if (!ret && errors)
    {
      rec_buf_puts (_("invalid date."), errors);
    }

  return ret;
}

static bool
rec_type_check_email (rec_type_t type,
                      const char *str,
                      rec_buf_t errors)
{
  bool ret;

  ret = rec_match (str, REC_TYPE_EMAIL_VALUE_RE);
  if (!ret && errors)
    {
      rec_buf_puts (_("invalid email."), errors);
    }

  return ret;
}

static bool
rec_type_check_enum (rec_type_t type,
                     const char *str,
                     rec_buf_t errors)
{
  size_t i;
  const char *p, *b;
  char name[100];

  if (rec_match (str, REC_TYPE_ENUM_VALUE_RE))
    {
      /* Get the name from STR.  */
      p = str;
      
      while (p && rec_blank_p (*p))
        {
          p++;
        }
      
      b = p;
      while (p && (rec_letter_p (*p)
                   || rec_letter_p (*p)
                   || rec_digit_p (*p)
                   || (*p == '_')
                   || (*p == '-')))
        {
          name[p - b] = *p;
          p++;
        }
      name[p - b] = '\0';
      
      /* Check for the name in the enum types.  */
      for (i = 0; i < type->size; i++)
        if (strcmp (name, type->data.names[i]) == 0)
          return true;
    }

  if (errors)
    {
      rec_buf_puts (_("invalid enum value."), errors);
    }

  return false;
}

static const char *
rec_type_parse_size (const char *str, rec_type_t type)
{
  const char *p;
  int size;

  p = str;

  /* Skip blanks.  */
  rec_skip_blanks (&p);

  /* Get the size.  */
  if (rec_parse_int (&p, &size)
      && (size >= 0))
    {
      type->data.max_size = size;
    }
  else
    {
      p = NULL;
    }

  return p;
}

static const char *
rec_type_parse_enum (const char *str, rec_type_t type)
{
  const char *p;
  size_t i;
  
  p = str;

  type->size = 0;
  type->data.names = NULL;

  while (*p)
    {
      /* Allocate space in the list of enum names if needed.  */
      if ((type->size % REC_ENUM_ALLOC_NAMES) == 0)
        type->data.names =
          realloc (type->data.names, ((type->size / REC_ENUM_ALLOC_NAMES) + 1) * (sizeof(char *) * REC_ENUM_ALLOC_NAMES));
      
      /* Skip blanks.  */
      rec_skip_blanks (&p);

      if (*p == '(')
        {
          /* Skip the comment.  */
          p++;
          while (*p && (*p != ')') && (*p != '('))
            {
              p++;
            }
          if (*p == ')')
            {
              p++;
            }
          else
            {
              /* Parse error: unterminated comment. */
              p = NULL;
              break;
            }
        }
      else if (*p)
        {
          /* Parse an enum entry.  */
          if (!rec_parse_regexp (&p,
                                 "^" REC_TYPE_ENUM_NAME_RE,
                                 &(type->data.names[type->size++])))
            {
              p = NULL;
              break;
            }
        }
    }

  if (type->size == 0)
    {
      /* We require at least one entry in the enum.  In this case it
         is not needed to save memory.  */
      return NULL;
    }

  if (!p)
    {
      /* Free memory.  */
      for (i = 0; i < type->size; i++)
        {
          free (type->data.names[i]);
        }
    }

  return p;
}

static const char *
rec_type_parse_regexp_type (const char *str, rec_type_t type)
{
  const char *p;
  char re[200];
  bool end_regexp;
  size_t i;
  char delim_char;

  p = str;

  /* The regexp type descriptor is like:
           
     BLANKS BEGIN_RE CHARS END_RE BLANKS
           
     where BEGIN_RE == END_RE and is the first non-blank
     character found in the string.  Escaped apperances of
     BEGIN_RE in CHARS are un-escaped.
  */

  /* Skip blanks.  */
  rec_skip_blanks (&p);
        
  end_regexp = false;
  delim_char = *p;
  p++;
        
  i = 0;
  while (*p)
    {
      if (*p == delim_char)
        {
          if (*(p + 1) == delim_char)
            {
              re[i++] = delim_char;
              p++;
            }
          else
            {
              /* End of the regexp.  */
              p++;
              end_regexp = true;
              break;
            }
        }
      else
        {
          re[i++] = *p;
        }
            
      p++;
    }
  re[i] = '\0';
        
  if (!end_regexp)
    {
      /* Error.  */
      p = NULL;
    }
  else
    {
      /* Compile the regexp.  */
      if (regcomp (&type->data.regexp, re,
                   REG_EXTENDED) != 0)
        {
          p = NULL;
        }
    }

  return p;
}

static const char *
rec_type_parse_rec (const char *str, rec_type_t type)
{
  const char *p = str;
  
  /* Get the record name.  */
  rec_skip_blanks (&p);
  if (!rec_parse_regexp (&p,
                         "^" REC_RECORD_TYPE_RE,
                         &(type->data.recname)))
    {
      return NULL;
    }

  return p;
}

static bool
rec_type_parse_range_point (const char **str, int *num)
{
  if (rec_match (*str, "^MIN"))
    {
      *num = INT_MIN;
      *str += 3;
    }
  else if (rec_match (*str, "^MAX"))
    {
      *num = INT_MAX;
      *str += 3;
    }
  else if (!rec_parse_int (str, num))
    {
      return false;
    }

  return true;
}

static const char *
rec_type_parse_range (const char *str, rec_type_t type)
{
  const char *p;

  p = str;

  rec_skip_blanks (&p);

  if (!rec_type_parse_range_point (&p, &(type->data.range[0])))
    {
      return NULL;
    }

  rec_skip_blanks (&p);

  if (*p == '\0')
    {
      /* One of the indexes is ommitted.  The range is of the
         form 0..N.  */
      type->data.range[1] = type->data.range[0];
      type->data.range[0] = 0;
    }
  else
    {
      if (!rec_type_parse_range_point (&p, &(type->data.range[1])))
        {
          return NULL;
        }
    }

  return p;
}

/* End of rec-types.c */
