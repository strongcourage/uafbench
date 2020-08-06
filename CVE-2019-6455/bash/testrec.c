/* -*- mode: C -*- Time-stamp: "2019-01-03 09:44:20 jemarch"
 *
 *       File:         testrec.c
 *       Date:         Fri Aug 23 21:41:00 2013
 *
 *       GNU recutils - testrec bash loadable builtin.
 *
 */

/* Copyright (C) 2013-2019 Jose E. Marchesi */

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

#include <bash/config.h>
#include <unistd.h>

#include <stdio.h>
#include <rec.h>

#include "builtins.h"
#include "shell.h"
#include "common.h"
#include "builtins/bashgetopt.h"

/* The function implementing the builtin.  It uses internal_getopt to
   parse options.  It is the same as getopt(3), but it takes a pointer
   to a WORD_LIST.

   If the builtin takes no options, call no_options(list) before doing
   anything else.  If it returns a non-zero value, your builtin should
   immediately return EX_USAGE.

   A builtin command returns EXECUTION_SUCCESS for success and
   EXECUTION_FAILURE to indicate failure.  */
int
testrec_builtin (WORD_LIST *list)
{
  int res = EXECUTION_SUCCESS;
  const char **argv;
  int argc;
  SHELL_VAR *var;
  rec_record_t record;
  rec_parser_t parser;
  char *record_str, *sex_str;

  /* Get arguments and verify them.  */

  argv = make_builtin_argv (list, &argc);
  if ((argc != 3)
      || ((strcmp (argv[0], "[%") == 0) && ((strlen (argv[2]) == 2) && ((argv[2][0] != '%') || (argv[2][1] != ']')))))
    {
      fprintf (stderr, "Usage: [%% SEX %%]\n");
      return EXECUTION_FAILURE;
    }

  sex_str = argv[1];

  /* Get the record to operate on from the REPLY_REC environment
     variable.  */
  var = find_variable ("REPLY_REC");
  if (!var)
    return EXECUTION_FAILURE;

  record_str = get_variable_value (var);
  parser = rec_parser_new_str (record_str, "REPLY_REC");
  if (!parser || !rec_parse_record (parser, &record))
    {
      fprintf (stderr, "testrec: error: invalid record in REPLY_REC\n");
      return EXECUTION_FAILURE;
    }

  /* Apply the selection expression.  */
  {
    bool status = false;
    rec_sex_t sex = rec_sex_new (false);
    if (!sex)
      return EXECUTION_FAILURE;

    if (!rec_sex_compile (sex, sex_str))
      {
        fprintf (stderr, "testrec: error: wrong selection expression\n");
        return EXECUTION_FAILURE;
      }

    res = rec_sex_eval (sex, record, &status) ? EXECUTION_SUCCESS : EXECUTION_FAILURE;
    rec_sex_destroy (sex);
  }

  /* Cleanup.  */
  rec_record_destroy (record);

  return res;
}

/* An array of strings forming the `long' documentation for the builtin,
   which is printed by `help xxx'.  It must end with a NULL.  By convention,
   the first line is a short description. */
char *testrec_doc[] = {
  "Evaluate a selection expression on the record stored in REPLY_REC.",
  "",
  "Evaluates a given selection expression on the record stored in the\n\
REPLY_REC variable, if any.\n\
\n\
Exit Status:\n\
The return code is zero if the selection expression evaluates to\n\
true, -1 otherwise.\n",
  (char *) NULL
};

/* The standard structure describing a builtin command.  bash keeps an
   array of these structures.  The flags must include BUILTIN_ENABLED
   so the builtin can be used. */
struct builtin testrec_struct = {
	"[%",	                /* builtin name */
	testrec_builtin,	/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	testrec_doc,		/* array of long documentation strings. */
	"testrec",		/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
