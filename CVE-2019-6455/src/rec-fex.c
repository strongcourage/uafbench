/* -*- mode: C -*-
 *
 *       File:         rec-fex.c
 *       Date:         Sun Apr 11 21:31:32 2010
 *
 *       GNU recutils - Field Expressions
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
#include <string.h>
#include <gettext.h>
#define _(str) dgettext (PACKAGE, str)

#include <rec-utils.h>
#include <rec.h>

/*
 * Data types.
 */

struct rec_fex_elem_s
{
  char *str;

  char *field_name;
  char *rewrite_to;
  int max;
  int min;

  char *function_name;
  void *function_data;
};

#define REC_FEX_MAX_ELEMS 256

struct rec_fex_s
{
  int num_elems;
  char *str;
  rec_fex_elem_t elems[REC_FEX_MAX_ELEMS];
};

/*
 * Static function declarations.
 */

static void rec_fex_init (rec_fex_t fex);

static bool rec_fex_parse_str_simple     (rec_fex_t new,       const char *str, const char *sep);
static bool rec_fex_parse_str_subscripts (rec_fex_t new,       const char *str);
static bool rec_fex_parse_elem           (rec_fex_elem_t elem, const char *str);

/*
 * Public functions.
 */

rec_fex_t
rec_fex_new (const char *str,
             enum rec_fex_kind_e kind)
{
  rec_fex_t new;
  int i;

  new = malloc (sizeof (struct rec_fex_s));
  if (new)
    {
      rec_fex_init (new);

      new->num_elems = 0;
      new->str = NULL;
      for (i = 0; i < REC_FEX_MAX_ELEMS; i++)
        {
          new->elems[i] = 0;
        }

      if (str != NULL)
        {
          /* Parse the string, using the proper parsing routine
             depending on the kind of field expression requested by
             the user.  */

          if (kind == REC_FEX_SUBSCRIPTS)
            {
              if (!rec_fex_parse_str_subscripts (new, str))
                {
                  /* Out of memory or parse error.  */
                  free (new);
                  return NULL;
                }
            }
          else if (kind == REC_FEX_SIMPLE)
            {
              if (!rec_fex_parse_str_simple (new, str, " \t\n"))
                {
                  /* Out of memory or parse error.  */
                  free (new);
                  return NULL;
                }
            }
          else /* REC_FEX_CSV */
            {
              if (!rec_fex_parse_str_simple (new, str, ","))
                {
                  /* Out of memory or parse error.  */
                  free (new);
                  return NULL;
                }
            }
        }
    }
  
  return new;
}

void
rec_fex_destroy (rec_fex_t fex)
{
  int i;
  
  if (fex)
    {
      for (i = 0; i < fex->num_elems; i++)
        {
          free (fex->elems[i]->rewrite_to);
          free (fex->elems[i]->field_name);
          free (fex->elems[i]->str);
          free (fex->elems[i]);
        }
      
      free (fex->str);
      free (fex);
    }
}

rec_fex_t
rec_fex_dup (rec_fex_t fex)
{
  rec_fex_t copy = NULL;
  size_t i = 0;

  copy = malloc (sizeof (struct rec_fex_s));
  if (copy)
    {
      rec_fex_init (copy);
  
      copy->num_elems = fex->num_elems;
      copy->str = strdup (fex->str);
      if (!copy->str)
        {
          /* Out of memory.  */
          rec_fex_destroy (copy);
          return NULL;
        }

      for (i = 0; i < fex->num_elems; i++)
        {
          if (fex->elems[i] == NULL)
            {
              copy->elems[i] = NULL;
              continue;
            }

          copy->elems[i] = malloc (sizeof (struct rec_fex_elem_s));
          if (!copy->elems[i])
            {
              /* Out of memory.  */
              rec_fex_destroy (copy);
              return NULL;
            }

          copy->elems[i]->max = fex->elems[i]->max;
          copy->elems[i]->min = fex->elems[i]->min;

#define REC_COPY_STR_MAYBE_RETURN(FNAME)                                \
          do                                                            \
            {                                                           \
               if (!fex->elems[i]->FNAME)                               \
                 {                                                      \
                   copy->elems[i]->FNAME = NULL;                        \
                 }                                                      \
               else                                                     \
                 {                                                      \
                   copy->elems[i]->FNAME = strdup (fex->elems[i]->FNAME); \
                   if (!copy->elems[i]->FNAME)                          \
                     {                                                  \
                       /* Out of memory.  */                            \
                       rec_fex_destroy (copy);                          \
                       return NULL;                                     \
                     }                                                  \
                 }                                                      \
            } while (0)

          REC_COPY_STR_MAYBE_RETURN (str);
          REC_COPY_STR_MAYBE_RETURN (field_name);
          REC_COPY_STR_MAYBE_RETURN (rewrite_to);
          REC_COPY_STR_MAYBE_RETURN (function_name);
        }
    }

  return copy;
}

bool
rec_fex_check (const char *str, enum rec_fex_kind_e kind)
{
  char *regexp_str;

  switch (kind)
    {
    case REC_FEX_SIMPLE:
      {
        regexp_str = "^" REC_FNAME_LIST_RE "$";
        break;
      }
    case REC_FEX_CSV:
      {
        regexp_str = "^" REC_FNAME_LIST_CS_RE "$";
        break;
      }
    case REC_FEX_SUBSCRIPTS:
      {
        regexp_str = "^" REC_FNAME_LIST_SUB_RE "$";
        break;
      }
    default:
      {
        regexp_str = NULL;
        break;
      }
    }

  return rec_match (str, regexp_str);
}

size_t
rec_fex_size (rec_fex_t fex)
{
  return fex->num_elems;
}

rec_fex_elem_t
rec_fex_get (rec_fex_t fex,
             size_t position)
{
  if ((position < 0) || (position >= fex->num_elems))
    {
      return NULL;
    }

  return fex->elems[position];
}

const char *
rec_fex_elem_field_name (rec_fex_elem_t elem)
{
  return elem->field_name;
}

bool
rec_fex_elem_set_field_name (rec_fex_elem_t elem,
                             const char *fname)
{
  free (elem->field_name);
  elem->field_name = strdup (fname);
  return (elem->field_name != NULL);
}

int
rec_fex_elem_min (rec_fex_elem_t elem)
{
  return elem->min;
}

int
rec_fex_elem_max (rec_fex_elem_t elem)
{
  return elem->max;
}

const char *
rec_fex_elem_rewrite_to (rec_fex_elem_t elem)
{
  return elem->rewrite_to;
}

void
rec_fex_sort (rec_fex_t fex)
{
  bool done;
  rec_fex_elem_t aux;
  int i, j;

  /* XXX: this code only works when 'max' is not specified.  */

  for (i = 1; i < fex->num_elems; i++)
    {
      aux = fex->elems[i];
      j = i - 1;
      done = false;

      while (!done)
       {
         /* If elems[j] > aux  */
         if ((fex->elems[j]->min == -1) || (fex->elems[j]->min > aux->min))
           {
             fex->elems[j + 1] = fex->elems[j];
             j = j - 1;
             if (j < 0)
               {
                 done = true;
               }
           }
         else
           {
             done = true;
           }
       }

      fex->elems[j + 1] = aux;
    }
}

char *
rec_fex_str (rec_fex_t fex,
             enum rec_fex_kind_e kind)
{
  char *result;
  size_t result_size;
  rec_buf_t buf;
  size_t i;
  char *tmp;

  result = NULL;
  buf = rec_buf_new (&result, &result_size);
  if (buf)
    {
      char *field_str = NULL;

      for (i = 0; i < fex->num_elems; i++)
        {
          if (i != 0)
            {
              if (kind == REC_FEX_SIMPLE)
                {
                  rec_buf_putc (' ', buf);
                }
              else
                {
                  rec_buf_putc (',', buf);
                }
            }
          
          field_str = strdup (fex->elems[i]->field_name);
          if (!field_str)
            {
              rec_buf_close (buf);
              free (result);
              return NULL;
            }

          rec_buf_puts (field_str, buf);
          free (field_str);

          if (kind == REC_FEX_SUBSCRIPTS)
            {
              if ((fex->elems[i]->min != -1)
                  || (fex->elems[i]->max != -1))
                {
                  rec_buf_putc ('[', buf);
                  if (fex->elems[i]->min != -1)
                    {
                      if (asprintf (&tmp, "%d", fex->elems[i]->min) != -1)
                        {
                          rec_buf_puts (tmp, buf);
                          free (tmp);
                        }
                    }
                  if (fex->elems[i]->max != -1)
                    {
                      if (asprintf (&tmp, "-%d", fex->elems[i]->max) != -1)
                        {
                          rec_buf_puts (tmp, buf);
                          free (tmp);
                        }
                    }

                  rec_buf_putc (']', buf);
                }
            }
        }
    }

  rec_buf_close (buf);

  return result;
}

bool
rec_fex_member_p (rec_fex_t fex,
                  const char *fname,
                  int min,
                  int max)
{
  bool res = false;
  int i;
  
  for (i = 0; i < fex->num_elems; i++)
    {
      if (rec_field_name_equal_p (fname,
                                  fex->elems[i]->field_name)
          && ((min == -1) || (fex->elems[i]->min == min))
          && ((max == -1) || (fex->elems[i]->max == max)))
        {
          res = true;
          break;
        }
    }

  return res;
}

rec_fex_elem_t
rec_fex_append (rec_fex_t fex,
                const char *fname,
                int min,
                int max)
{
  rec_fex_elem_t new_elem;

  if (fex->num_elems >= REC_FEX_MAX_ELEMS)
    {
      fprintf (stderr, _("internal error: REC_FEX_MAX_ELEMS exceeded.  Please report this.\n"));
      return NULL;
    }

  new_elem = malloc (sizeof (struct rec_fex_elem_s));
  if (new_elem)
    {
      memset (new_elem, 0, sizeof (*new_elem));
      new_elem->field_name = strdup (fname);
      if (!new_elem->field_name)
        {
          /* Out of memory.  */
          free (new_elem);
          return NULL;
        }

      new_elem->str = strdup (fname);
      if (!new_elem->str)
        {
          /* Out of memory.  */
          free (new_elem->field_name);
          free (new_elem);
          return NULL;
        }

      new_elem->min = min;
      new_elem->max = max;
      fex->elems[fex->num_elems++] = new_elem;
    }

  return new_elem;
}

const char *
rec_fex_elem_function_name (rec_fex_elem_t elem)
{
  return elem->function_name;
}

void **
rec_fex_elem_function_data (rec_fex_elem_t elem)
{
  return elem->function_data;
}

bool
rec_fex_all_calls_p (rec_fex_t fex)
{
  bool result = true;
  size_t i = 0;

  for (i = 0; i < fex->num_elems; i++)
    {
      if (fex->elems[i]->function_name == NULL)
        {
          result = false;
          break;
        }
    }

  return result;
}

/*
 * Private functions.
 */

static void
rec_fex_init (rec_fex_t fex)
{
  /* Initialize the field expression structure so it can be safely
     passed to rec_fex_destroy even if its contents are not completely
     initialized with real values.  */

  memset (fex, 0 /* NULL */, sizeof (struct rec_fex_s));
}

static bool
rec_fex_parse_str_simple (rec_fex_t new,
                          const char *str,
                          const char *sep)
{
  bool res;
  rec_fex_elem_t elem;
  char *fex_str, *fex_str_orig;
  char *elem_str;
  size_t i;

  if (!str)
    {
      return false;
    }

  fex_str = strdup (str);
  if (!fex_str)
    {
      return false;
    }
  fex_str_orig = fex_str;

  res = true;

  elem_str = strsep (&fex_str, sep);
  do
    {
      if (strlen (elem_str) > 0)
        {
          if ((elem = malloc (sizeof (struct rec_fex_elem_s))))
            {
              const char *p = elem_str;

              /* Get the field name.  */

              if (!rec_parse_regexp (&p,
                                     "^" REC_FNAME_RE,
                                     &(elem->field_name)))
                {
                  /* Parse error. */
                  free (elem);
                  res = false;
                  break;
                }

              /* Get the subname, if any, and modify the name
                 accordingly.  */

              if (*p == '.')
                {
                  char *subname = NULL;

                  p++;
                  if (!rec_parse_regexp (&p,
                                         "^" REC_FNAME_RE,
                                         &subname))
                    {
                      /* Parse error.  */
                      free (elem->field_name);
                      free (elem);
                      res = false;
                      break;
                    }

                  /* Concatenate the field name and the subname.  */
                  elem->field_name = rec_concat_strings (elem->field_name, "_", subname);
                }

              /* Check that there are no extra stuff at the end of the
                 string.  */

              if (*p != '\0')
                {
                  free (elem->field_name);
                  free (elem);
                  res = false;
                  break;
                }

              /* Initialize other attributes of the fex entry.  */

              elem->function_name = NULL;
              elem->function_data = NULL;
              elem->rewrite_to = NULL;
              elem->str = strdup (elem_str);
              elem->min = -1;
              elem->max = -1;
              new->elems[new->num_elems++] = elem;
            }
          else
            {
              res = false;
              break;
            }
        }
    }
  while ((elem_str = strsep (&fex_str, sep)));

  if (new->num_elems == 0)
    {
      /* No elements were recognized.  */
      res = false;
    }

  if (res)
    {
      new->str = strdup (str);
    }
  else
    {
      /* Destroy parsed elements.  */
      for (i = 0; i < new->num_elems; i++)
        {
          free (new->elems[i]->rewrite_to);
          free (new->elems[i]->field_name);
          free (new->elems[i]->str);
          free (new->elems[i]);
        }
    }

  free (fex_str_orig);
  return res;
}

static bool
rec_fex_parse_str_subscripts (rec_fex_t new,
                              const char *str)
{
  bool res;
  char *elem_str;
  char *fex_str, *fex_str_orig;
  rec_fex_elem_t elem;
  int i;

  res = true;

  fex_str = strdup (str);
  if (!fex_str)
    {
      return false;
    }
  fex_str_orig = fex_str;

  elem_str = strsep (&fex_str, ",");
  do
    {
      elem = malloc (sizeof (struct rec_fex_elem_s));
      if (!elem)
        {
          /* Out of memory.  */
          res = false;
          break;
        }

      if (!rec_fex_parse_elem (elem, elem_str))
        {
          /* Parse error.  */
          for (i = 0; i < new->num_elems; i++)
            {
              free (new->elems[i]->field_name);
              free (new->elems[i]->str);
              free (new->elems[i]);
            }

          free (elem);
          res = false;
          break;
        }

      /* Add the elem to the FEX.  */
      new->elems[new->num_elems++] = elem;
    }
  while ((elem_str = strsep (&fex_str, ",")));

  if (res)
    {
      new->str = strdup (str);
    }

  free (fex_str_orig);
  return res;
}

static bool
rec_fex_parse_elem (rec_fex_elem_t elem,
                    const char *str)
{
  bool ret;
  const char *p;

  ret = true;
  p = str;

  /* 'Empty' part.  */
  elem->field_name = NULL;
  elem->function_name = NULL;
  elem->function_data = NULL;
  elem->str = NULL;
  elem->rewrite_to = NULL;
  elem->min = -1;
  elem->max = -1;

  /* The 'str' field keeps a copy of the textual entry.  */

  elem->str = strdup (str);

  /* Each FEX element can be either a function call or a field name
     with an optional subscript.  */

  if (rec_match (p, "^" REC_FEX_CALL))
    {
      /* Get the function name and the field argument and store them
         in the FEX element.  */

      if (!rec_parse_regexp (&p,
                             "^" REC_FEX_FUNCTION_NAME,
                             &(elem->function_name)))
        {
          /* Parse error.  */
          free (elem->str);
          return false;
        }

      p++; /* Skip the ( */
    }

  /* Get the field name.  */
      
  if (!rec_parse_regexp (&p,
                         "^" REC_FNAME_RE,
                         &(elem->field_name)))
    {
      /* Parse error.  */
      free (elem->str);
      return false;
    }
  
  /* Get the subname and modify the name accordingly, if it
     exists.  */
  
  if (*p == '.')
    {
      char *subname = NULL;
      
      p++;
      if (!rec_parse_regexp (&p,
                             "^" REC_FNAME_RE,
                             &subname))
        {
          /* Parse error.  */
          free (elem->str);
          return false;
        }
      
      /* Concatenate the field_name and the subname.  */
      elem->field_name = rec_concat_strings (elem->field_name, "_", subname);
    }
  
  /* Get the subscripts if they are present.  */
  if (*p == '[')
    {
      p++;
      /* First subscript in range.  */
      if (!rec_parse_int (&p, &(elem->min)))
        {
          /* Parse error.  */
          free (elem->str);
          free (elem->field_name);
          return false;
        }
      
      if (*p == '-')
        {
          p++;
          /* Second subscript in range.  */
          if (!rec_parse_int (&p, &(elem->max)))
            {
              /* Parse error.  */
              free (elem->str);
              free (elem->field_name);
              return false;
            }
        }
      
      if (*p != ']')
        {
          /* Parse error.  */
          free (elem->str);
          free (elem->field_name);
          return false;
        }
      p++; /* Skip the ]  */
    }

  if (elem->function_name)
    {
      p++; /* Skip the ) */
    }
      
  /* Get the rewrite rule if it is present.  */
  if (*p == ':')
    {
      p++;
      if (!rec_parse_regexp (&p,
                             "^" REC_FNAME_RE,
                             &(elem->rewrite_to)))
        {
          /* Parse error.  */
          free (elem->str);
          free (elem->field_name);
          return false;
        }
    }

  if (*p != '\0')
    {
      free (elem->str);
      free (elem->field_name);
      free (elem->rewrite_to);
      return false;
    }

  return ret;
}

/* End of rec-fex.c */
