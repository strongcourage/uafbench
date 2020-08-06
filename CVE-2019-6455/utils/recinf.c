/* -*- mode: C -*-
 *
 *       File:         recinf.c
 *       Date:         Mon Dec 28 08:54:38 2009
 *
 *       GNU recutils - recinf
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

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <xalloc.h>
#include <gettext.h>
#define _(str) gettext (str)

#include <rec.h>
#include <recutl.h>

/*
 * Global variables
 */

rec_writer_mode_t recinf_write_mode = REC_WRITER_NORMAL;

/*
 * Command line options management
 */

enum
{
  COMMON_ARGS,
  DESCRIPTOR_ARG,
  NAMES_ARG,
  TYPE_ARG,
  PRINT_SEXPS_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"descriptor", no_argument, NULL, DESCRIPTOR_ARG},
    {"names-only", no_argument, NULL, NAMES_ARG},
    {"type", required_argument, NULL, TYPE_ARG},
    {"print-sexps", no_argument, NULL, PRINT_SEXPS_ARG},
    {NULL, 0, NULL, 0}
  };

/*
 * Global variables.
 */

bool recinf_descriptor = false;
bool recinf_names_only = false;
char *recinf_type = NULL;

/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recinf synopsis.
     no-wrap */
  printf (_("\
Usage: recinf [OPTION]... [FILE]...\n"));

  /* TRANSLATORS: --help output, recinf short description.
     no-wrap */
  fputs (_("\
Print information about the types of records stored in the input.\n"),
         stdout);

  puts ("");
  /* TRANSLATORS: --help output, recinf arguments.
     no-wrap */
  fputs (_("\
  -t, --type=RECORD_TYPE          print information on the records having the\n\
                                    specified type.\n\
  -d, --descriptor                include the full record descriptors.\n\
  -n, --names-only                output just the names of the record files\n\
                                    found in the input.\n"),
         stdout);

  recutl_print_help_common ();

  puts ("");
  /* TRANSLATORS: --help output, recinf special options.
     no-wrap */
  fputs (_("\
Special options:\n\
  -S, --print-sexps                   print the data in sexps instead of rec format.\n"),
         stdout);
  
  puts ("");
  recutl_print_help_footer ();
}

bool
print_info_file (FILE *in,
                 char *file_name)
{
  bool ret;
  rec_db_t db;
  rec_rset_t rset;
  rec_record_t descriptor;
  rec_parser_t parser;
  int position;

  ret = true;
  parser = rec_parser_new (in, file_name);

  ret = rec_parse_db (parser, &db);
  if (ret)
    {
      for (position = 0; position < rec_db_size (db); position++)
        {
          rset = rec_db_get_rset (db, position);
          descriptor = rec_rset_descriptor (rset);

          if (recinf_type
              && descriptor
              && (strcmp (rec_rset_type (rset), recinf_type) != 0))
            {
              continue;
            }

          if (recinf_descriptor)
            {
              rec_writer_t writer;
              
              if (descriptor)
                {
                  writer = rec_writer_new (stdout);
                  rec_writer_set_mode (writer, recinf_write_mode);
                  rec_write_record (writer, descriptor);
                  rec_write_string (writer, "\n");
                  rec_writer_destroy (writer);
                }
              else
                {
                  if (recinf_write_mode == REC_WRITER_NORMAL)
                    {
                      printf ("unknown\n");
                    }
                }

              if (position < (rec_db_size (db) - 1))
                {
                  printf ("\n");
                }
            }
          else
            {
              if (descriptor)
                {
                  if (!recinf_names_only)
                    {
                      fprintf (stdout, "%zd ", rec_rset_num_records (rset));
                    }
                  fprintf (stdout, "%s\n", rec_rset_type (rset));
                }
              else
                {
                  if (!recinf_names_only)
                    {
                      printf ("%zd\n", rec_rset_num_records (rset));
                    }
                }
            }          
        }
    }
  
  if (rec_parser_error (parser))
    {
      rec_parser_perror (parser, file_name);
    }

  rec_parser_destroy (parser);

  return ret;
}

int
main (int argc, char *argv[])
{
  char c;
  int ret;
  char *file_name;
  FILE *in;

  recutl_init ("recinf");

  while ((ret = getopt_long (argc,
                             argv,
                             "Sdnt:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
        case PRINT_SEXPS_ARG:
        case 'S':
          {
            recinf_write_mode = REC_WRITER_SEXP;
            break;
          }
        case DESCRIPTOR_ARG:
        case 'd':
          {
            recinf_descriptor = true;
            break;
          }
        case NAMES_ARG:
        case 'n':
          {
            recinf_names_only = true;
            break;
          }
        case TYPE_ARG:
        case 't':
          {
            recinf_type = xstrdup (optarg);
            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }

  /* Process the input files, if any.  Otherwise use the standard
     input to read the rec data. */
  if (optind < argc)
    {
      while (optind < argc)
        {
          file_name = argv[optind++];
          if (!(in = fopen (file_name, "r")))
            {
              printf(_("error: cannot read file %s\n"), file_name);
              exit (EXIT_FAILURE);
            }
          else
            {
              if (!print_info_file (in, file_name))
                {
                  /* Parse error */
                  exit (EXIT_FAILURE);
                }

              fclose (in);
            }
        }
    }
  else
    {
      if (!print_info_file (stdin, "stdin"))
        {
          /* Parse error */
          exit (EXIT_FAILURE);
        }
    }
  
  return EXIT_SUCCESS;
}

/* End of recinf.c */
