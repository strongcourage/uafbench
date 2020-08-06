/* -*- mode: C -*-
 *
 *       File:         rec-writer.c
 *       Date:         Sat Dec 26 22:47:16 2009
 *
 *       GNU recutils - Writer
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

#include <rec.h>

#include <stdlib.h>
#include <string.h>

#include <rec.h>
#include <rec-utils.h>

/*
 * Static functions defined in this file
 */
static bool rec_writer_putc (rec_writer_t writer, char c);
static bool rec_writer_puts (rec_writer_t writer, const char *s);

/* Writer Data Structure
 *
 */
struct rec_writer_s
{
  FILE *file_out;    /* File stream used by the writer. */
  rec_buf_t buf_out; /* Growable buffer used by the writer. */

  bool eof;
  int line;  /* Current line number. */

  /* The following flags and options can be accesssed using the
     corresponding rec_writer_(get/set)_FLAG function.  */

  bool collapse_p;        /* If true the writer won't introduce
                             separators between records.  */
  bool skip_comments_p;   /* If true the writer won't print out
                             comments.  */
  enum rec_writer_mode_e mode; /* The mode in which the writer operates.
                                  See the definition of the enumerated type
                                  in rec.h for a list of allowed modes.  */

};

static void
rec_writer_new_common (rec_writer_t writer)
{
  writer->file_out = NULL;
  writer->buf_out = NULL;
  writer->line = 1;
  writer->eof = false;
  writer->collapse_p = false;
  writer->skip_comments_p = false;
  writer->mode = REC_WRITER_NORMAL;
}

rec_writer_t
rec_writer_new (FILE *file_out)
{
  rec_writer_t new;

  new = malloc (sizeof(struct rec_writer_s));
  if (new)
    {
      rec_writer_new_common (new);
      new->file_out = file_out;
    }

  return new;
}

rec_writer_t
rec_writer_new_str (char **str, size_t *str_size)
{
  rec_writer_t new;

  new = malloc (sizeof(struct rec_writer_s));
  if (new)
    {
      rec_writer_new_common (new);
      new->buf_out = rec_buf_new (str, str_size);
    }

  return new;
}

void
rec_writer_destroy (rec_writer_t writer)
{
  if (writer)
    {
      if (writer->file_out)
        {
          fflush (writer->file_out);
        }
      if (writer->buf_out)
        {
          rec_buf_close (writer->buf_out);
        }

      free (writer);
    }
}

bool
rec_write_comment (rec_writer_t writer,
                   rec_comment_t comment)
{
  char *line;
  char *str;
  char *orig_str;
  size_t i;
  
  if (writer->mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, "(comment "))
        {
          return false;
        }
      if (!rec_writer_putc (writer, '"'))
        {
          return false;
        }

      str = rec_comment_text (comment);
      for (i = 0; i < strlen (str); i++)
        {
          if (str[i] == '\n')
            {
              if (!rec_writer_puts (writer, "\\n"))
                {
                  return false;
                }
            }
          else
            {
              if (!rec_writer_putc (writer, str[i]))
                {
                  return false;
                }
            }
        }

      if (!rec_writer_puts (writer, "\")"))
        {
          return false;
        }
    }
  else
    {
      /* Every line in the comment is written preceded by a '#'
         character.  The lines composing the comments are separated by
         newline characters.  */

      bool first = true;
      
      str = strdup (rec_comment_text (comment));
      orig_str = str; /* Save a pointer to str to deallocate it later,
                         since strsep will modify the str
                         variable.  */
      line = strsep (&str, "\n");
      do
        {
          if (!first)
            {
              if (!rec_writer_putc (writer, '\n'))
                {
                  return false;
                }
            }

          if (!rec_writer_putc (writer, '#') 
              || !rec_writer_puts (writer, line))
            {
              return false;
            }

          first = false;
        }
      while ((line = strsep (&str, "\n")));

      free (orig_str);
    }

  return true;
}

bool
rec_write_field (rec_writer_t writer,
                 rec_field_t field)
{
  size_t pos;
  const char *fname;
  const char *fvalue;
  enum rec_writer_mode_e mode = writer->mode;

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, "(field "))
        {
          return false;
        }
      if (!rec_writer_puts (writer, rec_field_char_location_str (field)))
        {
          return false;
        }
      if (!rec_writer_putc (writer, ' '))
        {
          return false;
        }
    }

  if ((mode != REC_WRITER_VALUES) && (mode != REC_WRITER_VALUES_ROW))
    {
      fname = rec_field_name (field);
      if (!rec_write_field_name (writer, fname))
        {
          return false;
        }
    }
  
  /* Write the field value */
  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_putc (writer, ' '))
        {
          return false;
        }
  
      if (!rec_writer_putc (writer, '"'))
        {
          return false;
        }
    }

  fvalue = rec_field_value (field);

  if ((strlen (fvalue) > 0) && (mode == REC_WRITER_NORMAL))
    {
      if (!rec_writer_putc (writer, ' '))
        {
          return false;
        }
    }

  for (pos = 0; pos < strlen (fvalue); pos++)
    {
      if (fvalue[pos] == '\n')
        {
          if (mode == REC_WRITER_SEXP)
            {
              if (!rec_writer_puts (writer, "\\n"))
                {
                  return false;
                }
            }
          else if (mode == REC_WRITER_NORMAL)
            {
              if (!rec_writer_puts (writer, "\n+ "))
                {
                  return false;
                }
            }
          else
            {
              if (!rec_writer_putc (writer, '\n'))
                {
                  return false;
                }
            }
        }
      else if (((fvalue[pos] == '"') || (fvalue[pos] == '\\')) && (mode == REC_WRITER_SEXP))
        {
          if ((!rec_writer_putc (writer, '\\'))
              || (!rec_writer_putc (writer, fvalue[pos])))
            {
              return false;
            }
        }
      else
        {
          if (!rec_writer_putc (writer, fvalue[pos]))
            {
              /* EOF on output */
              return false;
            }
        }
    }

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_putc (writer, '"'))
        {
          return false;
        }
    }

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, ")"))
        {
          return false;
        }
    }

  return true;
}

bool
rec_write_field_name (rec_writer_t writer,
                      const char *field_name)
{
  /* Field names can be written in several formats, according to the
   * desired mode:
   *
   * REC_WRITER_NORMAL
   *    The field name is written in rec format. i.e. NP:
   * REC_WRITER_SEXP
   *    The field name is a string: "NP"
   */

  enum rec_writer_mode_e mode = writer->mode;

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_putc (writer, '"'))
        {
          return false;
        }
    }

  if (!rec_writer_puts (writer, field_name))
    {
      return false;
    }

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_putc (writer, '"'))
        {
          return false;
        }
    }
  else
    {
      if (!rec_writer_putc (writer, ':'))
        {
          return false;
        }
    }

  return true;
}

bool
rec_write_record (rec_writer_t writer,
                  rec_record_t record)
{
  bool ret;
  rec_mset_iterator_t iter;
  rec_mset_elem_t elem;
  char *data;
  size_t num_field, num_elem, num_fields, num_elems;
  enum rec_writer_mode_e mode = writer->mode;

  ret = true;

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, "(record "))
        return false;
      if (!rec_writer_puts (writer, rec_record_char_location_str (record)))
        return false;
      if (!rec_writer_puts (writer, " (\n"))
        return false;
    }

  num_elems = rec_record_num_elems (record);
  num_fields = rec_record_num_fields (record);
  num_field = 0;
  num_elem = 0;
  iter = rec_mset_iterator (rec_record_mset (record));
  while (rec_mset_iterator_next (&iter, MSET_ANY, (const void **) &data, &elem))
    {
      if (rec_mset_elem_type (elem) == MSET_FIELD)
        {
          /* Write a field.  */
          rec_field_t field = (rec_field_t) data;

          if (!rec_write_field (writer, field))
            {
              ret = false;
              break;
            }

          /* Include a field separator.  */

          if ((mode == REC_WRITER_VALUES_ROW) 
              && (num_field != (num_fields - 1)))
            {
              if(mode == REC_WRITER_VALUES_ROW)
                {
                  if (!rec_writer_putc (writer, ' '))
                    return false;
                }
            }
          else if ((writer->skip_comments_p && (num_field != (num_fields - 1)))
                   || (!writer->skip_comments_p && (num_elem != (num_elems - 1))))
            {
              if (!rec_writer_putc (writer, '\n'))
                return false;
            }

          num_field++;
        }
      else if (!writer->skip_comments_p)
        {
          /* Write a comment.  */

          rec_comment_t comment = (rec_comment_t) data;

          if ((mode != REC_WRITER_VALUES) && (mode != REC_WRITER_VALUES_ROW))
            {
              if (!rec_write_comment (writer, comment))
                {
                  ret = false;
                  break;
                }

              if (num_elem != (num_elems - 1))
                {
                  if (!rec_writer_putc (writer, '\n'))
                    return false;
                }
            }
        }

      num_elem++;
    }

  rec_mset_iterator_free (&iter);

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, "))"))
        {
          return false;
        }
    }

  return ret;
}

bool
rec_write_rset (rec_writer_t writer,
                rec_rset_t rset)
{
  bool ret;
  rec_record_t descriptor;
  bool wrote_descriptor;
  size_t position;
  size_t descriptor_pos;
  rec_mset_iterator_t iter;
  rec_mset_elem_t elem;
  void *data;
  enum rec_writer_mode_e mode = writer->mode;
  
  ret = true;
  wrote_descriptor = false;
  position = 0;
  descriptor_pos = rec_rset_descriptor_pos (rset);
  descriptor = rec_rset_descriptor (rset);

  /* Special case: record set containing just the record
     descriptor.  */
  if ((rec_rset_num_elems (rset) == 0) && descriptor)
    {
      rec_write_record (writer,
                        rec_rset_descriptor (rset));
      rec_writer_putc (writer, '\n');

      return true;
    }

  iter = rec_mset_iterator (rec_rset_mset (rset));
  while (rec_mset_iterator_next (&iter, MSET_ANY, (const void **) &data, &elem))
    {
      if (position != 0)
        {
          if (!rec_writer_putc (writer, '\n'))
            {
              ret = false;
            }
        }

      if (position == descriptor_pos)
        {
          if (descriptor 
              && (!(wrote_descriptor = rec_write_record (writer,
                                                         rec_rset_descriptor (rset)))))
            {
              ret = false;
            }
          else
            {
              if (wrote_descriptor)
                {
                  if (!rec_writer_puts (writer, "\n\n"))
                    {
                      ret = false;
                    }
                }
            }
        }
      
      if (rec_mset_elem_type (elem) == MSET_RECORD)
        {
          ret = rec_write_record (writer, (rec_record_t) data);
        }
      else if (!writer->skip_comments_p)
        {
          ret = rec_write_comment (writer, (rec_comment_t) data);
        }

      if (!writer->collapse_p || (position == (rec_rset_num_elems (rset) - 1)))
        {
          if (!rec_writer_putc (writer, '\n'))
            {
              ret = false;
            }
        }
      
      if (!ret)
        {
          break;
        }
      
      position++;
    }

  rec_mset_iterator_free (&iter);

  /* Special case:
   *
   * # comment 1
   * 
   * # comment 2
   * ...
   * %rec: foo
   */
  if (!wrote_descriptor
      && (descriptor_pos >= rec_rset_num_elems (rset))
      && rec_rset_descriptor (rset))
    {
      if (!rec_writer_putc (writer, '\n'))
        {
          ret = false;
        }
      if (!rec_write_record (writer, rec_rset_descriptor (rset)))
        {
          ret = false;
        }
      if (!rec_writer_putc (writer, '\n'))
        {
          ret = false;
        }
    }

  return ret;
}

bool
rec_write_db (rec_writer_t writer,
              rec_db_t db)
{
  bool ret;
  int i;

  ret = true;
  for (i = 0; i < rec_db_size (db); i++)
    {
      rec_rset_t rset = rec_db_get_rset (db, i);

      if (i != 0)
        {
          if (!rec_writer_putc (writer, '\n'))
            {
              ret = false;
              break;
            }
        }
      
      if (!rec_write_rset (writer, rset))
        {
          ret = false;
          break;
        }
    }

  return ret;
}

char *
rec_write_field_str (rec_field_t field,
                     rec_writer_mode_t mode)
{
  rec_writer_t writer;
  char *result;
  size_t result_size;
  
  result = NULL;
  writer = rec_writer_new_str (&result, &result_size);
  if (writer)
    {
      rec_writer_set_mode (writer, mode);
      rec_write_field (writer, field);
      rec_writer_destroy (writer);
    }
  
  return result;
}

char *
rec_write_field_name_str (const char *field_name,
                          rec_writer_mode_t mode)
{
  rec_writer_t writer;
  char *result;
  size_t result_size;
  
  result = NULL;
  writer = rec_writer_new_str (&result, &result_size);
  if (writer)
    {
      rec_writer_set_mode (writer, mode);
      rec_write_field_name (writer, field_name);
      rec_writer_destroy (writer);
    }
  
  return result;
}

char *
rec_write_comment_str (rec_comment_t comment,
                       rec_writer_mode_t mode)
{
  rec_writer_t writer;
  char *result;
  size_t result_size;
  
  result = NULL;
  writer = rec_writer_new_str (&result, &result_size);
  if (writer)
    {
      rec_writer_set_mode (writer, mode);
      rec_write_comment (writer, comment);
      rec_writer_destroy (writer);
    }
  
  return result;
}

bool
rec_write_string (rec_writer_t writer,
                  const char *str)
{
  return rec_writer_puts (writer, str);
}

void
rec_writer_set_collapse (rec_writer_t writer,
                         bool value)
{
  writer->collapse_p = value;
}

void
rec_writer_set_skip_comments (rec_writer_t writer,
                              bool value)
{
  writer->skip_comments_p = value;
}

void
rec_writer_set_mode (rec_writer_t writer,
                     enum rec_writer_mode_e mode)
{
  writer->mode = mode;
}

/*
 * Private functions
 */

static bool
rec_writer_putc (rec_writer_t writer, char c)
{
  bool ret;

  ret = false;
  if (writer->file_out)
    {
      ret = (fputc (c, writer->file_out) != EOF);
    }
  if (writer->buf_out)
    {
      ret = (rec_buf_putc (c, writer->buf_out) != EOF);
    }

  return ret;
}

static bool
rec_writer_puts (rec_writer_t writer, const char *s)
{
  bool ret;

  ret = false;
  if (writer->file_out)
    {
      ret = (fputs (s, writer->file_out) != EOF);
    }
  if (writer->buf_out)
    {
      ret = (rec_buf_puts (s, writer->buf_out) != EOF);
    }

  return ret;
}

/* End of rec-writer.c */
