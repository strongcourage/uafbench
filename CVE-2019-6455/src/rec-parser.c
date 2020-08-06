/* -*- mode: C -*-
 *
 *       File:         rec-parser.c
 *       Date:         Wed Dec 23 20:55:15 2009
 *
 *       GNU recutils - Parsing routines
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
#include <stdarg.h>
#include <gettext.h>
#define _(str) dgettext (PACKAGE, str)
#define N_(str) gettext_noop (str)

#include <rec.h>
#include <rec-utils.h>

/*
 * Static functions defined in this file
 */
static int rec_parser_getc (rec_parser_t parser);
static int rec_parser_ungetc (rec_parser_t parser, int ci);

static bool rec_expect (rec_parser_t parser, const char *str);

static bool rec_parse_field_value (rec_parser_t parser, char **str);

static bool rec_parse_comment (rec_parser_t parser, rec_comment_t *comment);

static bool rec_parser_digit_p (char c);
static bool rec_parser_letter_p (char c);
static bool rec_parser_init_common (rec_parser_t parser, const char *source);

/*
 * Parser Data Structure
 */

enum rec_parser_error_e
{
  REC_PARSER_NOERROR,
  REC_PARSER_ERROR,
  REC_PARSER_EUNGETC,
  REC_PARSER_EFNAME,
  REC_PARSER_ENOMEM,
  REC_PARSER_ETOOMUCHNAMEPARTS,
  REC_PARSER_ECOMMENT,
  REC_PARSER_EFIELD,
  REC_PARSER_ERECORD
};

struct rec_parser_s
{
  FILE *in_file;          /* File stream used by the parser.  */
  const char *in_buffer;  /* Buffer used by the parser.  */
  size_t in_size;         /* Length of in_buffer. */
  const char *p;          /* Pointer to the next unreaded character in
                             in_buffer */
  char *source;
  
  rec_record_t prev_descriptor;

  bool eof;
  enum rec_parser_error_e error;

  size_t line; /* Current line number. */
  size_t character; /* Current offset from the beginning of the file,
                       in characters.  */
};

const char *rec_parser_error_strings[] =
{
  "no error (unused)",
  "unknown error",
  "unreading a character",
  "expected a field name",
  "out of memory",
  "too much parts in field name",
  "expected a comment",
  "expected a field",
  "expected a record",
  NULL /* Sentinel */
};

/* The following macro is used by some functions to reduce
   verbosity.  */

#define FNAME(id) rec_std_field_name ((id))

/*
 * Public functions.
 */

rec_parser_t
rec_parser_new (FILE *in,
                const char *source)
{
  rec_parser_t parser;

  parser = malloc (sizeof (struct rec_parser_s));
  if (parser != NULL)
    {
      parser->in_file = in;
      parser->in_buffer = NULL;
      parser->in_size = 0;

      if (!rec_parser_init_common (parser, source))
        {
          free (parser);
          parser = NULL;
        }
    }

  return parser;
}

rec_parser_t
rec_parser_new_str (const char *buffer,
                    const char *source)
{
  return rec_parser_new_mem (buffer, strlen (buffer), source);
}

rec_parser_t
rec_parser_new_mem (const char *buffer,
                    size_t size,
                    const char *source)
{
  rec_parser_t parser;

  parser = malloc (sizeof (struct rec_parser_s));
  if (parser != NULL)
    {
      parser->in_buffer = buffer;
      parser->in_size = size;
      parser->in_file = NULL;

      if (!rec_parser_init_common (parser, source))
        {
          free (parser);
          parser = NULL;
        }
    }

  return parser;
}

void
rec_parser_destroy (rec_parser_t parser)
{
  if (parser)
    {
      free (parser->source);
      free (parser);
    }
}

bool
rec_parser_eof (rec_parser_t parser)
{
  return parser->eof;
}

bool
rec_parser_error (rec_parser_t parser)
{
  return (parser->error != REC_PARSER_NOERROR);
}

void
rec_parser_perror (rec_parser_t parser,
                   const char *fmt,
                   ...)
{
  va_list ap;
  char *number_str;

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  fputs (": ", stderr);
  number_str = NULL;  /* asprintf does the allocation */
  if (asprintf (&number_str, "%zu", parser->line) != -1)
    {
      fputs (number_str, stderr);
      free (number_str);
    }
  fputs (": error: ", stderr);
  fputs (gettext (rec_parser_error_strings[parser->error]), stderr);
  fputc ('\n', stderr);

  va_end (ap);
}

bool
rec_parse_field_name (rec_parser_t parser,
                      char **fname)
{
  bool ret = true;
  int ci;
  size_t str_size;
  char c;
  rec_buf_t buf;

  /* Sanity check */
  if (rec_parser_eof (parser)
      || rec_parser_error (parser))
    return false;

  buf = rec_buf_new (fname, &str_size);
  if (!buf)
    {
      /* Out of memory */
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }

  /* The syntax of a field name is described by the following regexp:
   *
   * [a-zA-Z%][a-zA-Z0-9_]*:
   */

  /* [a-zA-Z%] */
  ci = rec_parser_getc (parser);
  if (ci == EOF)
    ret = false;
  else
    {
      c = (char) ci;

      if ((rec_parser_letter_p (c))
          || (c == '%'))
        {
          if (rec_buf_putc (c, buf) == EOF)
            {
              /* Out of memory */
              parser->error = REC_PARSER_ENOMEM;
              return false;
            }
        }
      else
        {
          /* Parse error */
          parser->error = REC_PARSER_EFNAME;
          ret = false;
        }
    }

  /* [a-zA-Z0-9_]* */
  if (ret)
    {
      while ((ci = rec_parser_getc (parser)) != EOF)
        {
          c = (char) ci;

          if (rec_parser_letter_p (c)
              || rec_parser_digit_p (c)
              || (c == '_'))
            {
              if (rec_buf_putc (c, buf) == EOF)
                {
                  /* Out of memory */
                  parser->error = REC_PARSER_ENOMEM;
                  return false;
                }
              if (parser->error > 0)
                break;
            }
          else if (c == ':')
            /* End of token.  Consume the ':' and report success */
            break;
          else
            {
              /* Parse error */
              parser->error = REC_PARSER_EFNAME;
              ret = false;
              break;
            }
        }

      if (parser->eof)
        {
          parser->error = REC_PARSER_EFNAME;
          ret = false;
        }
    }

  rec_buf_close (buf);

  if (!ret)
    free (*fname);
  else
    {
      /* Field names ends with:
       *
       * - A blank character or
       * - A tab or
       * - A newline or
       * - The end of the file
       *
       * Note that if the field name ends with a newline it is
       * pushed back to the input stream, since (unlike a blank
       * character) it will be part of the field value.
       */

      ci = rec_parser_getc (parser);
      if (ci != EOF)
        {
          c = (char) ci;
          if ((c == ' ') || (c == '\t'))
            parser->error = REC_PARSER_NOERROR;
          else if (c == '\n')
            {
              parser->error = REC_PARSER_NOERROR;
              rec_parser_ungetc (parser, c);
            }
          else
            rec_parser_ungetc (parser, c);
        }
    }
  
  return ret;
}

void
rec_parser_reset (rec_parser_t parser)
{
  parser->eof = false;
  parser->error = REC_PARSER_NOERROR;
  parser->p = parser->in_buffer;
}

bool
rec_parse_field (rec_parser_t parser,
                 rec_field_t *field)
{
  bool ret;
  rec_field_t new;
  char *field_name;
  char *field_value;
  size_t location;
  size_t char_location;

  /* Sanity check */
  if (rec_parser_eof (parser)
      || rec_parser_error (parser))
    return false;

  location = parser->line;
  char_location = parser->character;
  if (char_location != 0)
    char_location++;

  ret = rec_parse_field_name (parser, &field_name);
  if (ret)
    {
      ret = rec_parse_field_value (parser, &field_value);

      if (ret)
        {
          new = rec_field_new (field_name,
                               field_value);
	  if (new == NULL)
	    return false;

          rec_field_set_source (new, parser->source);
          rec_field_set_location (new, location);
          rec_field_set_char_location (new, char_location);
          *field = new;

          free (field_value);
        }

      free (field_name);
    }

  return ret;
}

bool
rec_parse_record (rec_parser_t parser,
                  rec_record_t *record)
{
  rec_record_t new;
  rec_field_t field;
  bool ret;
  int ci;
  char c;
  rec_comment_t comment;
  size_t char_location;

  /* Sanity check */
  if (rec_parser_eof (parser)
      || rec_parser_error (parser))
    return false;

  new = rec_record_new ();
  if (!new)
    {
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }

  /* Localize the potential record.  */
  rec_record_set_source (new, parser->source);
  rec_record_set_location (new, parser->line);
  char_location = parser->character;

  if (char_location != 0)
    char_location++;
  rec_record_set_char_location (new, char_location);

  /* A record is a list of mixed fields and comments, containing at
   * least one field starting it:
   *
   *  FIELD (FIELD|COMMENT)*
   */
  if (rec_parse_field (parser, &field))
    /* Add the field to the record */
    rec_mset_append (rec_record_mset (new), MSET_FIELD, (void *) field, MSET_ANY);
  else
    {
      /* Expected a field.  */
      parser->error = REC_PARSER_EFIELD;
      rec_record_destroy (new);
      *record = NULL;
      return false;
    }

  ret = true;
  while ((ci = rec_parser_getc (parser)) != EOF)
    {
      c = (char) ci;

      if (c == '#')
        {
          rec_parser_ungetc (parser, ci);
          if (rec_parse_comment (parser, &comment))
            /* Add the comment to the record.  */
            rec_mset_append (rec_record_mset (new), MSET_COMMENT, (void *) comment, MSET_ANY);
        }
      else if ((c == ' ') || (c == '\t'))
        {
          /* A line composed just by blanks acts like an end of record
             separator. */

          while ((ci != EOF) && ((c == ' ') || (c == '\t')))
            {
              ci = rec_parser_getc (parser);
              c = (char) ci;
            }

          if ((ci == EOF) || (c == '\n'))
            /* End of record */
            break;
          else
            {
              /* Parse error: field expected */
              parser->error = REC_PARSER_EFIELD;
              ret = false;
              break;
            }
        }
      else if (c == '\n')
        /* End of record */
        break;
      else
        {
          /* Try to parse a field */
          rec_parser_ungetc (parser, ci);
          if (rec_parse_field (parser, &field))
            /* Add the field to the record */
            rec_mset_append (rec_record_mset (new), MSET_FIELD, (void *) field, MSET_ANY);
          else
            {
              /* Parse error: field expected */
              parser->error = REC_PARSER_EFIELD;
              ret = false;
              break;
            }
        }
    }

  if (ret)
    *record = new;
  else
    {
      rec_record_destroy (new);
      *record = NULL;
    }

  return ret;
}

bool
rec_parse_rset (rec_parser_t parser,
                rec_rset_t *rset)
{
  bool ret;
  int ci;
  char c;
  rec_rset_t new;
  rec_record_t record;
  rec_comment_t comment;
  size_t comments_added = 0;

  ret = false;

  if ((new = rec_rset_new ()) == NULL)
    {
      /* Out of memory */
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }

  /* Set the descriptor for this record set.  */
  rec_rset_set_descriptor (new, parser->prev_descriptor);
  parser->prev_descriptor = NULL;

  while ((ci = rec_parser_getc (parser)) != EOF)
    {
      c = (char) ci;

      /* Skip newline characters and blanks.  */
      if ((c == '\n') || (c == ' ') || (c == '\t'))
        continue;
      /* Skip comments */
      else if (c == '#')
        {
          rec_parser_ungetc (parser, c);
          rec_parse_comment (parser, &comment);

          /* Add the comment to the record set.  */
          rec_mset_append (rec_rset_mset (new), MSET_COMMENT, (void *) comment, MSET_ANY);

          comments_added++;
        }
      else
        {
          /* Try to parse a record */
          rec_parser_ungetc (parser, c);
          if (rec_parse_record (parser, &record))
            {
              /* Check if the parsed record is a descriptor.  In that
                 case, set it as the previous descriptor in the parser
                 state and stop parsing.  In the special case where
                 the previous descriptor is NULL (we did not find a
                 descriptor yet) then record the position of the
                 descriptor as well.

                 Otherwise, add the record to the current record
                 set. */
              if (rec_record_field_p (record, FNAME(REC_FIELD_REC)))
                {
                  if ((rec_rset_num_records (new) == 0) &&
                      (!rec_rset_descriptor (new)))
                    {
                      /* Special case: the first record found in the
                         input stream is a descriptor. */
                      rec_rset_set_descriptor (new, record);
                      rec_rset_set_descriptor_pos (new, comments_added);
                    }
                  else
                    {
                      parser->prev_descriptor = record;
                      ret = true;
                      break;
                    }
                }
              else
                {
                  rec_record_set_container (record, new);
                  rec_mset_append (rec_rset_mset (new), MSET_RECORD, (void *) record, MSET_ANY);
                }
            }
          else
            {
              /* Parse error */
              parser->error = REC_PARSER_ERECORD;
              break;
            }
        }
    }

  if ((parser->error == REC_PARSER_NOERROR)
      && (rec_rset_descriptor (new)
          || (rec_rset_num_records (new) > 0)))
    ret = true;

  if (ret)
      *rset = new;
  else
    {
      rec_rset_destroy (new);
      *rset = NULL;
    }

  return ret;
}

bool
rec_parse_db (rec_parser_t parser,
              rec_db_t *db)
{
  bool ret;
  rec_rset_t rset;
  rec_db_t new;

  ret = true;

  new = rec_db_new ();
  if (!new)
    /* Out of memory.  */
    return false;

  while (rec_parse_rset (parser, &rset))
    {
      /* Add the rset into the database.  */
      if (!rec_db_insert_rset (new,
                               rset,
                               rec_db_size (new)))
        {
          /* Parse error: out of memory.  */
          parser->error = REC_PARSER_ENOMEM;
          ret = false;
          break;
        }
    }

  if (!rec_parser_eof (parser))
    ret = false;

  if (ret)
    *db = new;

  return ret;
}

char *
rec_parse_field_name_str (const char *str)
{
  rec_parser_t parser;
  char *field_name = NULL;
  char *str2;
  size_t str_size;

  /* Make sure the input string ends with a colon character.  */
  str_size = strlen (str);
  str2 = malloc (str_size + 2);
  if (!str2)
    /* Out of memory.  */
    return NULL;

  memcpy (str2, str, str_size);
  if (str_size > 0 && str2[str_size - 1] == ':')
    str2[str_size] = '\0';
  else
    {
      str2[str_size] = ':';
      str2[str_size + 1] = '\0';
    }
  
  parser = rec_parser_new_str (str2, "dummy");
  if (!rec_parse_field_name (parser, &field_name))
    field_name = NULL;

  if (!rec_parser_eof (parser))
    {
      /* There is additional stuff after the field name.  */
      free (field_name);
      field_name = NULL;
    }

  rec_parser_destroy (parser);
  free (str2);

  return field_name;
}

rec_record_t
rec_parse_record_str (const char *str)
{
  rec_parser_t parser;
  rec_record_t record;

  record = NULL;
  parser = rec_parser_new_str (str, "dummy");
  if (parser)
    {
      if (!rec_parse_record (parser, &record))
        record = NULL;
      rec_parser_destroy (parser);
    }

  return record;
}

bool
rec_parser_seek (rec_parser_t parser,
                 size_t line_number,
                 size_t position)
{
  if (parser->in_file)
    {
      if (fseek (parser->in_file, position, SEEK_SET))
        return false;
    }
  else if (parser->in_buffer)
    {
      if (position > parser->in_size)
        return false;
      parser->p = parser->in_buffer + position;
    }
  else
    {
      /* This point should not be reached!  */
      fprintf (stderr, "rec_parser_seek: no backend in parser. This is a bug.\
  Please report it.");
      return false;
    }
  parser->line = line_number;
  parser->character = position;
  return true;
}

long
rec_parser_tell (rec_parser_t parser)
{
  if (parser->in_file)
    return ftell (parser->in_file);
  else if (parser->in_buffer)
    return parser->p - parser->in_buffer;
  else
    {
      /* This point should not be reached!  */
      fprintf (stderr, "rec_parser_seek: no backend in parser. This is a bug.\
  Please report it.");
      return -1;
    }
}

/*
 * Private functions
 */

static int
rec_parser_getc (rec_parser_t parser)
{
  int ci;

  /* Get the input character depending on the backend used (memory or
     file).  */
  if (parser->in_file)
    ci = getc (parser->in_file);
  else if (parser->in_buffer)
    {
      if (parser->p == parser->in_buffer + parser->in_size)
        ci = EOF;
      else
        {
          ci = *(parser->p);
          parser->p++;
        }
    }
  else
    {
      /* This point should not be reached!  */
      fprintf (stderr, "rec_parser_getc: no backend in parser. This is a bug.\
  Please report it.");
      return EOF;
    }

  /* Manage EOF and update statistics.  */

  if (ci == EOF)
    parser->eof = true;
  else 
    {
      parser->character++;
      if (((char) ci) == '\n')
        parser->line++;
    }

  return ci;
}

int
rec_parser_ungetc (rec_parser_t parser,
                   int ci)
{
  int res;

  /* Update statistics.  */

  parser->character--;
  if (((char) ci) == '\n')
    parser->line--;

  /* Unread the character, depending on the backend used (memory or
     file).  */

  if (parser->in_file)
    {
      res = ungetc (ci, parser->in_file);
      if (res != ci)
        parser->error = REC_PARSER_EUNGETC;
    }
  else if (parser->in_buffer)
    {
      if (parser->p > parser->in_buffer)
        {
          res = ci; /* Emulate ungetc. */
          parser->p--;
        }
      else
        {
          res = EOF;
          parser->error = REC_PARSER_EUNGETC;
        }
    }
  else
    {

      /* This point should not be reached!  */
      fprintf (stderr, "rec_parser_ungetc: no backend in parser. This is a bug.\
  Please report it.");
      return EOF;
    }

  return res;
}

static bool
rec_parser_digit_p (char c)
{
  return ((c >= '0') && (c <= '9'));
}

static bool
rec_parser_letter_p (char c)
{
  return (((c >= 'A') && (c <= 'Z'))
          || ((c >= 'a') && (c <= 'z')));
}

static bool
rec_expect (rec_parser_t parser,
            const char *str)
{
  size_t str_size;
  size_t counter;
  bool found;
  int ci;
  char c;

  found = true;
  str_size = strlen (str);

  for (counter = 0;
       counter < str_size;
       counter++)
    {
      ci = rec_parser_getc (parser);
      if (ci == EOF)
        {
          /* EOF */
          found = false;
          parser->eof = true;
          break;
        }
      else
        {
          c = (char) ci;
          if (c != str[counter])
            {
              /* Not match */
              rec_parser_ungetc (parser, ci);
              found = false;
              break;
            }
        }
    }

  return found;
}

static bool
rec_parse_field_value (rec_parser_t parser,
                       char **str)
{
  bool ret;
  int ci, ci2;
  char c, c2;
  size_t str_size;
  bool prev_newline;
  rec_buf_t buf;

  /* Sanity check */
  if (rec_parser_eof (parser)
      || rec_parser_error (parser))
    return false;

  c = '\0';
  prev_newline = false;
  ret = true;
  buf = rec_buf_new (str, &str_size);
  if (!buf)
    {
      /* Out of memory */
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }
  
  /* A field value is a sequence of zero or more ascii codes finished
   * with a newline character.
   *
   *  \$ is translated to nothing.
   *  $+ ? is translated to $.
   */
  while ((ci = rec_parser_getc (parser)) != EOF)
    {
      c = (char) ci;

      if ((prev_newline) && (c != '+'))
        {
          /* End of value */
          rec_parser_ungetc (parser, ci);
          rec_buf_rewind (buf, 1);
          break;
        }

      if (c == '\\')
        {
          ci2 = rec_parser_getc (parser);
          if (ci2 == EOF)
            {
              parser->eof = true;
              ret = false;
              break;
            }
          else
            {
              c2 = (char) ci2;
              if (c2 == '\n')
                {
                  /* Consume both $\n chars not adding them to str =>
                     do nothing here. */
                }
              else
                {
                  /* Add \ and put back c2 */
                  if (rec_buf_putc (c, buf) == EOF)
                    {
                      /* Out of memory */
                      parser->error = REC_PARSER_ENOMEM;
                      return false;
                    }

                  if (parser->error > 0)
                    {
                      break;
                    }

                  if (rec_parser_ungetc (parser, ci2)
                      != ci2)
                    {
                      ret = false;
                      break;
                    }
                }
            }

          prev_newline = false;
        }
      else if (c == '+')
        {
          if (prev_newline)
            {
              /* Reduce \n+ ? to \n by ignoring the + ? */
              ci2 = rec_parser_getc (parser);
              
              if (ci2 == EOF)
                {
                  parser->eof = true;
                  ret = false;
                  break;
                }
              else
                {
                  c2 = (char) ci2;
                  /* If the look ahead character is a blank, skip it.
                     Otherwise put it back in the stream so it will be
                     processed in the next iteration. */
                  if (c2 != ' ')
                    {
                      if (rec_parser_ungetc (parser, ci2) != ci2)
                        {
                          ret = false;
                          break;
                        }
                    }
                }
            }
          else
            {
              if (rec_buf_putc (c, buf) == EOF)
                {
                  /* Out of memory */
                  parser->error = REC_PARSER_ENOMEM;
                  return false;
                }

              if (parser->error > 0)
                break;
            }

          prev_newline = false;
        }
      else if (c == '\n')
        {
          if (rec_buf_putc (c, buf) == EOF)
            {
              /* Out of memory */
              parser->error = REC_PARSER_ENOMEM;
              return false;
            }

          if (parser->error > 0)
            break;
          prev_newline = true;
        }
      else
        {
          if (rec_buf_putc (c, buf) == EOF)
            {
              /* Out of memory */
              parser->error = REC_PARSER_ENOMEM;
              return false;
            }

          if (parser->error > 0)
            break;
          prev_newline = false;
        }
    }

  if (ret)
    {
      if (rec_parser_eof (parser) && (c == '\n'))
        /* Special case: field just before EOF */
        rec_buf_rewind (buf, 1);
    }

  rec_buf_close (buf);

  if (!ret)
    free (*str);

  return ret;
}

static bool
rec_parse_comment (rec_parser_t parser, rec_comment_t *comment)
{
  bool ret;
  rec_buf_t buf;
  char *str;
  size_t str_size;
  int ci;
  char c;

  ret = false;
  buf = rec_buf_new (&str, &str_size);

  /* Comments start at the beginning of line and span until the first
   * \n character not followed by a #, or EOF.
   */
  if (rec_expect (parser, "#"))
    {
      while ((ci = rec_parser_getc (parser)) != EOF)
        {
          c = (char) ci;

          if (c == '\n')
            {
              if ((ci = rec_parser_getc (parser)) == EOF)
                break;
              c = (char) ci;

              if (c != '#')
                {
                  rec_parser_ungetc (parser, ci);
                  break;
                }
              else
                c = '\n';
            }

          if (rec_buf_putc (c, buf) == EOF)
            {
              /* Out of memory */
              parser->error = REC_PARSER_ENOMEM;
              return false;
            }
        }
      
      ret = true;
    }

  rec_buf_close (buf);

  if (ret)
    *comment = rec_comment_new (str);
  else
    *comment = NULL;

  free (str);
  return ret;
}

static bool
rec_parser_init_common (rec_parser_t parser,
                        const char *source)
{
  if (source)
    {
      parser->source = strdup (source);
      if (!parser->source)
        return false;
    }
  else
    parser->source = NULL;

  parser->eof = false;
  parser->error = REC_PARSER_NOERROR;
  parser->line = 1;
  parser->character = 0;
  parser->prev_descriptor = NULL;
  parser->p = parser->in_buffer;

  return true;
}

/* End of rec-parser.c */
