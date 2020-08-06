/* -*- mode: C -*-
 *
 *       File:         rec-sex-parser.c
 *       Date:         Tue Jan 12 18:01:37 2010
 *
 *       GNU recutils - Sexy parser
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
#include <stdio.h>

#include <rec-sex-parser.h>
#include "rec-sex-tab.h"
/*#include "rec-sex-lex.h" */

/*
 * Data types
 */

struct rec_sex_parser_s
{
  char *in;              /* String to be parsed.  */
  size_t index;          /* Index in in_str.  */
  void *scanner;         /* Flex scanner.  */
  bool case_insensitive;

  rec_sex_ast_t ast;
};

/*
 * Public functions
 */

rec_sex_parser_t
rec_sex_parser_new (void)
{
  rec_sex_parser_t new;

  new = malloc (sizeof (struct rec_sex_parser_s));
  if (new)
    {
      new->in = NULL;
      new->index = 0;
      new->case_insensitive = false;

      /* Initialize the sexy scanner.  */
      sexlex_init (&(new->scanner));
      sexset_extra (new, new->scanner);
    }

  return new;
}

void *
rec_sex_parser_scanner (rec_sex_parser_t parser)
{
  return parser->scanner;
}

void
rec_sex_parser_destroy (rec_sex_parser_t parser)
{
  if (parser->scanner)
    {
      sexlex_destroy (parser->scanner);
    }

  free (parser->in);
  free (parser);
}

rec_sex_ast_t
rec_sex_parser_ast (rec_sex_parser_t parser)
{
  return parser->ast;
}

void
rec_sex_parser_set_ast (rec_sex_parser_t parser,
                        rec_sex_ast_t ast)
{
  parser->ast = ast;
}

bool
rec_sex_parser_case_insensitive (rec_sex_parser_t parser)
{
  return parser->case_insensitive;
}

void
rec_sex_parser_set_case_insensitive (rec_sex_parser_t parser,
                                     bool case_insensitive)
{
  parser->case_insensitive = case_insensitive;
}

void
rec_sex_parser_set_in (rec_sex_parser_t parser,
                       const char *str)
{
  if (parser->in)
    {
      free (parser->in);
      parser->in = NULL;
    }

  parser->in = strdup (str);
  parser->index = 0;
}

int
rec_sex_parser_getc (rec_sex_parser_t parser)
{
  int res;

  res = -1;
  if ((parser->in)
      && (parser->index < strlen (parser->in)))
    {
      res = parser->in[parser->index++];
    }

  return res;
}

bool
rec_sex_parser_run (rec_sex_parser_t parser,
                    const char *expr)
{
  int res;

  rec_sex_parser_set_in (parser, expr);
  if (!sexparse (parser))
    {
      res = true;
    }
  else
    {
      /* Parse error.  */
      res = false;
    }

  return res;
}

void
rec_sex_parser_print_ast (rec_sex_parser_t parser)
{
  rec_sex_ast_print (parser->ast);
}

/* End of rec-sex-parser.c */
