/* -*- mode: C -*-
 *
 *       File:         readrec.c
 *       Date:         Fri Aug 23 18:38:08 2013
 *
 *       GNU recutils - readrec bash loadable builtin.
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
readrec_builtin (WORD_LIST *list)
{
  SHELL_VAR *var;
  rec_parser_t parser;
  rec_record_t record;

  if (no_options (list) != 0)
    return EX_USAGE;

  /* Create a librec parser to operate on the standard input and try
     to read a record.  If there is a parse error then report it and
     fail.  */

  parser = rec_parser_new (stdin, "stdin");
  if (!parser)
    return EXECUTION_FAILURE;

  if (!rec_parse_record (parser, &record))
    {
      return EXECUTION_FAILURE;
    }

  {
    size_t record_str_size = 0;
    char *record_str = NULL;
    char *record_str_dequoted = NULL;
    rec_writer_t writer = rec_writer_new_str (&record_str, &record_str_size);

    if (!writer || !rec_write_record (writer, record))
      return EXIT_FAILURE;
    rec_writer_destroy (writer);

    /* Set the REPLY_REC environment variable to the read record.  */
    record_str_dequoted = dequote_string (record_str);
    var = bind_variable ("REPLY_REC", record_str_dequoted, 0);
    VUNSETATTR (var, att_invisible);
    xfree (record_str_dequoted);

    /* Set the environment variables for the fields.  */
    {
      rec_field_t field = NULL;
      rec_mset_iterator_t iter = rec_mset_iterator (rec_record_mset (record));

      //      rec_record_reset_marks (record);
      while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
        {
          char *var_name = rec_field_name (field);
          size_t num_fields = rec_record_get_num_fields_by_name (record, var_name);

          //          if (rec_record_field_mark (record, field))
          //            continue;

#if defined ARRAY_VARS
          if (num_fields > 1)
            {
              /* In case several fields share the same field name, create
                 an array variable containing all the values.  */

              size_t i = 0;
              for (; i < num_fields; i++)
                {
                  //                  rec_record_mark_field (record, field, true);
                  field = rec_record_get_field_by_name (record, var_name, i);
                  var = bind_array_variable (var_name, i, rec_field_value (field), 0);
                  VUNSETATTR (var, att_invisible);
                }
            }
          else
            {
              /* Bind a normal variable.  */
              char *var_value = rec_field_value (field);
              var = bind_variable (var_name, var_value, 0);
              VUNSETATTR (var, att_invisible);
            }
#endif /* ARRAY_VARS */
        }
      rec_mset_iterator_free (&iter);
    }
  }

  return EXECUTION_SUCCESS;
}

/* An array of strings forming the `long' documentation for the builtin,
   which is printed by `help xxx'.  It must end with a NULL.  By convention,
   the first line is a short description. */
char *readrec_doc[] = {
  "Read a recutils record from the standard input.",
  "",
  "The read record is stored in the REPLY_REC variable.  Additional variables",
  "are set named after the fields in the record.",
  "",
  "Exit Status:",
  "The return code is zero, unless end-of-file is encountered.",
  (char *) NULL
};

/* The standard structure describing a builtin command.  bash keeps an
   array of these structures.  The flags must include BUILTIN_ENABLED
   so the builtin can be used. */
struct builtin readrec_struct = {
	"readrec",		/* builtin name */
	readrec_builtin,	/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	readrec_doc,		/* array of long documentation strings. */
	"readrec",		/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
