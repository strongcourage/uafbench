/* -*- mode: C -*-
 *
 *       File:         rec-comment.c
 *       Date:         Wed Apr  7 17:22:45 2010
 *
 *       GNU recutils - Comments.
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

/*
 * Public functions.
 */

rec_comment_t
rec_comment_new (char *text)
{
  if (!text)
    return NULL;

  return strdup (text);
}

void
rec_comment_destroy (rec_comment_t comment)
{
  free (comment);
}

rec_comment_t
rec_comment_dup (rec_comment_t comment)
{
  return strdup (comment);
}

char *
rec_comment_text (rec_comment_t comment)
{
  return comment;
}

void
rec_comment_set_text (rec_comment_t *comment,
                      char *text)
{
  free (*comment);
  *comment = strdup (text);
}

bool
rec_comment_equal_p (rec_comment_t comment1,
                     rec_comment_t comment2)
{
  return (strcmp (comment1, comment2) == 0);
}

/* End of rec-comment.c */
