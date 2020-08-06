/* -*- mode: C -*-
 *
 *       File:         rec-utils.c
 *       Date:         Fri Apr  9 19:45:00 2010
 *
 *       GNU recutils - Miscellanea utilities
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

#include <regex.h>
#include <stdlib.h>
#include <gettext.h>
#define _(str) dgettext (PACKAGE, str)
#include <string.h>
#include <locale.h>

#include <rec-utils.h>

bool
rec_atoi (const char *str,
          int *number)
{
  bool res;
  long int li;
  char *end;
  int base = 0;

  res = false;

  li = strtol (str, &end, base);
  if ((*str != '\0') && (*end == '\0'))
    {
      /* The entire string is valid.  */
      res = true;
      *number = (int) li;
    }

  return res;
}

bool
rec_atod (const char *str,
          double *number)
{
  bool res;
  char *end;

  res = false;
  
  setlocale (LC_NUMERIC, "C"); /* We want the dot to always be the
                                  decimal separator. */
  *number = strtod (str, &end);
  setlocale (LC_NUMERIC, ""); /* Restore the locale from the
                                 environment.  */

  if ((*str != '\0') && (*end == '\0'))
    {
      /* The entire string is valid.  */
      res = true;
    }
  
  return res;
}

char *
rec_extract_file (const char *str)
{
  regex_t regexp;
  regmatch_t matches;
  char *rec_file = NULL;
  size_t rec_file_length = 0;

  if (regcomp (&regexp, "[ \n\t]" REC_FILE_REGEXP, REG_EXTENDED) != 0)
    {
      fprintf (stderr, _("internal error: rec_int_rec_extract_file: error compiling regexp.\n"));
      return NULL;
    }

  if ((regexec (&regexp, str, 1, &matches, 0) == 0)
      && (matches.rm_so != -1))
    {
      /* Get the match.  */
      rec_file_length = matches.rm_eo - matches.rm_so;
      rec_file = malloc (rec_file_length + 1);
      memcpy (rec_file, str + matches.rm_so + 1, rec_file_length - 1);
      rec_file[rec_file_length - 1] = '\0';
    }

  regfree (&regexp);
  return rec_file;
}

char *
rec_extract_url (const char *str)
{
  regex_t regexp;
  regmatch_t matches;
  char *rec_url = NULL;
  size_t rec_url_length = 0;

  if (regcomp (&regexp, REC_URL_REGEXP, REG_EXTENDED) != 0)
    {
      fprintf (stderr, _("internal error: rec_int_rec_extract_url: error compiling regexp.\n"));
      return NULL;
    }

  if ((regexec (&regexp, str, 1, &matches, 0) == 0)
      && (matches.rm_so != -1))
    {
      /* Get the match.  */
      rec_url_length = matches.rm_eo - matches.rm_so;
      rec_url = malloc (rec_url_length + 1);
      memcpy (rec_url, str + matches.rm_so, rec_url_length);
      rec_url[rec_url_length] = '\0';
    }

  regfree (&regexp);
  return rec_url;
}

char *
rec_extract_type (const char *str)
{
  regex_t regexp;
  regmatch_t matches;
  char *rec_type = NULL;
  size_t rec_type_length = 0;

  /* TODO: use a REC_TYPE_NAME_RE  */
  if (regcomp (&regexp, REC_FNAME_RE, REG_EXTENDED) != 0)
    {
      fprintf (stderr, _("internal error: rec_int_rec_extract_url: error compiling regexp.\n"));
      return NULL;
    }

  if ((regexec (&regexp, str, 1, &matches, 0) == 0)
      && (matches.rm_so != -1))
    {
      /* Get the match.  */
      rec_type_length = matches.rm_eo - matches.rm_so;
      rec_type = malloc (rec_type_length + 1);
      memcpy (rec_type, str + matches.rm_so, rec_type_length);
      rec_type[rec_type_length] = '\0';
    }

  regfree (&regexp);
  return rec_type;
}

bool
rec_parse_int (const char **str, int *num)
{
  bool ret;
  const char *p, *b;
  char number[30];

  ret = true;
  p = *str;

  b = p;
  while (rec_digit_p (*p)
         || ((p == b) && (*p == '-'))
         || ((*p >= 'a') && (*p <= 'f'))
         || ((*p >= 'A') && (*p <= 'F'))
         || (*p == 'x')
         || (*p == 'X'))
    {
      number[p - b] = *p;
      p++;
    }
  number[p - b] = '\0';

  if (!rec_atoi (number, num))
    {
      ret = false;
    }

  if (ret)
    {
      *str = p;
    }

  return ret;
}

bool
rec_parse_regexp (const char **str, const char *re, char **result)
{
  bool ret;
  const char *p;
  regex_t regexp;
  regmatch_t pm;

  ret = true;
  p = *str;

  /* Compile the regexp.  */
  if (regcomp (&regexp, re, REG_EXTENDED) != 0)
    {
      ret = false;
    }

  if (ret)
    {
      /* Try to match the regexp.  */
      if (regexec (&regexp, p, 1, &pm, 0) == 0)
        {
          if (result)
            {
              /* Get the match into 'result'.  Note that
                 since the pattern starts with a ^ rm_so shall be 0 and we
                 can use rm_eo relative to *p.  */
              *result = malloc (pm.rm_eo + 1);
              memcpy (*result, p, pm.rm_eo);
              (*result)[pm.rm_eo] = '\0';
            }

          /* Advance 'p'.  */
          p = p + pm.rm_eo;
        }
      else
        {
          ret = false;
          if (result)
            {
              *result = NULL;
            }
        }

      regfree (&regexp);
    }

  if (ret)
    {
      *str = p;
    }

  return ret;
}

void
rec_skip_blanks (const char **str)
{
  const char *p;

  p = *str;
  while (rec_blank_p (*p))
    {
      p++;
    }

  *str = p;
}

bool
rec_blank_p (char c)
{
  return ((c == ' ')
          || (c == '\n')
          || (c == '\t'));
}

bool
rec_digit_p (char c)
{
  return ((c >= '0') && (c <= '9'));
}

bool
rec_letter_p (char c)
{
  return (((c >= 'a') && (c <= 'z'))
          || ((c >= 'A') && (c <= 'Z')));
}

static bool
rec_match_int (const char *str,
               const char *reg,
               int flags)
{
  bool ret;
  regex_t regexp;

  if (regcomp (&regexp, reg, flags) != 0)
    {
      fprintf (stderr, _("internal error: rec_match: error compiling regexp.\n"));
      return false;
    }

  ret = (regexec (&regexp, str, 0, NULL, 0) == 0);
  regfree (&regexp);

  return ret;
}

bool
rec_match (const char *str,
           const char *reg)
{
  return rec_match_int (str, reg, REG_EXTENDED);
}

bool
rec_match_insensitive (const char *str,
                       const char *reg)
{
  return rec_match_int (str, reg, REG_EXTENDED | REG_ICASE);
}

size_t
rec_extract_size (const char *str)
{
  const char *p;
  char *condition_str;
  int res;

  if (!rec_match (str, REC_INT_SIZE_RE))
    {
      return 0;
    }

  p = str;
  rec_skip_blanks (&p);
  rec_parse_regexp (&p, "^[><]=?", &condition_str);
  rec_skip_blanks (&p);
  rec_parse_int (&p, &res);

  return (size_t) res;
}

enum rec_size_condition_e
rec_extract_size_condition (const char *str)
{
  const char *p;
  char *condition_str = NULL;
  enum rec_size_condition_e condition;

  if (!rec_match (str, REC_INT_SIZE_RE))
    {
      return SIZE_COND_E;
    }

  p = str;
  rec_skip_blanks (&p);
  rec_parse_regexp (&p, "^[><]=?", &condition_str);

  if (condition_str)
    {
      if (strcmp (condition_str, ">") == 0)
        {
          condition = SIZE_COND_G;
        }
      else if (strcmp (condition_str, ">=") == 0)
        {
          condition = SIZE_COND_GE;
        }
      else if (strcmp (condition_str, "<") == 0)
        {
          condition = SIZE_COND_L;
        }
      else if (strcmp (condition_str, "<=") == 0)
        {
          condition = SIZE_COND_LE;
        }
      else
        {
          fprintf (stderr, "internal error: rec_extract_size_condition: invalid condition.\n");
          return SIZE_COND_E;
        }

      free (condition_str);
    }
  else
    {
      condition = SIZE_COND_E;
    }

  return condition;
}

int
rec_timespec_subtract (struct timespec *result,
                       struct timespec *x,
                       struct timespec *y)
{
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;
  if (result->tv_nsec < 0)
    {
      /* Overflow.  Subtract one second.  */
      result->tv_sec--;
      result->tv_nsec += 1000000000;
    }

  /* Return whether there is an overflow in the 'tv_sec' field.  */
  return (result->tv_sec < 0);
}

uint32_t
rec_endian_swap (uint32_t number)
{
  uint32_t res;

  res = (number >> 24)
    | ((number << 8) & 0x00FF0000)
    | ((number >> 8) & 0x0000FF00)
    | (number << 24);

  return res;
}

char *
rec_concat_strings (const char *str1,
                    const char *str2,
                    const char *str3)
{
  char *res = malloc (strlen (str1) + strlen (str2) + strlen (str3) + 1);

  if (res)
    {
      memcpy (res, str1, strlen (str1));
      memcpy (res + strlen (str1), str2, strlen (str2));
      memcpy (res + strlen (str1) + strlen (str2), str3, strlen (str3) + 1);
    }

  return res;
}

/* End of rec-utils.c */
