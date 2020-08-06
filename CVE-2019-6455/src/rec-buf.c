/* -*- mode: C -*-
 *
 *       File:         rec-buf.c
 *       Date:         Fri Dec 17 18:40:53 2010
 *
 *       GNU recutils - Flexible buffers.
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

#include <rec.h>

#define REC_BUF_STEP 512

struct rec_buf_s
{
  char *data;
  size_t size;
  size_t used;

  /* Pointers to user-provided variables that will be updated at
     rec_buf_destroy time.  */
  char **data_pointer;
  size_t *size_pointer;
};

/*
 * Public functions.
 */

rec_buf_t
rec_buf_new (char **data, size_t *size)
{
  rec_buf_t new;

  new = malloc (sizeof (struct rec_buf_s));
  if (new)
    {
      new->data_pointer = data;
      new->size_pointer = size;

      new->data = malloc (REC_BUF_STEP);
      new->size = REC_BUF_STEP;
      new->used = 0;

      if (!new->data)
        {
          free (new);
          new = NULL;
        }
    }

  return new;
}

void
rec_buf_close (rec_buf_t buf)
{
  /* Adjust the buffer.  */
  if (buf->used > 0)
      buf->data = realloc (buf->data, buf->used + 1);
  buf->data[buf->used] = '\0';

  /* Update the user-provided buffer and size.  */
  *(buf->data_pointer) = buf->data;
  *(buf->size_pointer) = buf->used;

  /* Don't deallocate buf->data */
  free (buf);
}

void
rec_buf_rewind (rec_buf_t buf, int n)
{
  if ((buf->used - n) >= 0)
    buf->used = buf->used - n;
}

int
rec_buf_putc (int c, rec_buf_t buf)
{
  unsigned int ret;

  if (c == EOF)
    return EOF;

  ret = (unsigned int) c;
  if ((buf->used + 1) > buf->size)
    {
      /* Allocate a new block */
      buf->size = buf->size + REC_BUF_STEP;
      buf->data = realloc (buf->data, buf->size);

      if (!buf->data)
        /* Not enough memory.
           REC_BUF_STEP should not be 0. */
        ret = EOF;
    }

  if (ret != EOF)
    /* Add the character */
    buf->data[buf->used++] = (char) c;

  return ret;
}

int
rec_buf_puts (const char *str, rec_buf_t buf)
{
  int ret;
  const char *p;

  ret = 0;
  p = str;
  while (*p != '\0')
    {
      if (rec_buf_putc (*p, buf) == EOF)
        {
          /* Error.  */
          ret = -1;
          break;
        }

      ret++;
      p++;
    }

  return ret;
}

/* End of rec-buf.c */
