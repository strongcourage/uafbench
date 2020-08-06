/* -*- mode: C -*-
 *
 *       File:         recsel.c
 *       Date:         Fri Jan  1 23:12:38 2010
 *
 *       GNU recutils - recsel
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

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <xalloc.h>
#include <gettext.h>
#define _(str) gettext (str)

#include <rec.h>
#include <recutl.h>

/* Forward prototypes.  */
void recsel_parse_args (int argc, char **argv);
bool recsel_process_data (rec_db_t db);

/*
 * Global variables
 */

char      *recutl_sex_str      = NULL;
rec_sex_t  recutl_sex          = NULL;
char      *recutl_quick_str    = NULL;
char      *recsel_fex_str      = NULL;
rec_fex_t  recsel_fex          = NULL;
char      *recutl_type         = NULL;
bool       recsel_collapse     = false;
bool       recsel_count        = false;
bool       recutl_insensitive  = false;
bool       recsel_descriptors  = false;
rec_fex_t  recutl_sort_by_fields  = NULL;
rec_fex_t  recsel_group_by_fields = NULL;
rec_writer_mode_t recsel_write_mode = REC_WRITER_NORMAL;
char      *recsel_password     = NULL;
bool       recsel_uniq         = false;
size_t     recutl_random       = 0;
char      *recsel_join         = NULL;

/*
 * Command line options management.
 */

enum
{
  COMMON_ARGS,
  RECORD_SELECTION_ARGS,
  PRINT_ARG,
  PRINT_VALUES_ARG,
  PRINT_IN_A_ROW_ARG,
  COLLAPSE_ARG,
  COUNT_ARG,
  DESCRIPTOR_ARG,
  PRINT_SEXPS_ARG,
  SORT_ARG,
#if defined REC_CRYPT_SUPPORT
  PASSWORD_ARG,
#endif
  UNIQ_ARG,
  GROUP_BY_ARG,
  JOIN_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    RECORD_SELECTION_LONG_ARGS,
    {"print", required_argument, NULL, PRINT_ARG},
    {"print-values", required_argument, NULL, PRINT_VALUES_ARG},
    {"print-row", required_argument, NULL, PRINT_IN_A_ROW_ARG},
    {"collapse", no_argument, NULL, COLLAPSE_ARG},
    {"count", no_argument, NULL, COUNT_ARG},
    {"include-descriptors", no_argument, NULL, DESCRIPTOR_ARG},
    {"print-sexps", no_argument, NULL, PRINT_SEXPS_ARG},
    {"sort", required_argument, NULL, SORT_ARG},
#if defined REC_CRYPT_SUPPORT
    {"password", required_argument, NULL, PASSWORD_ARG},
#endif
    {"uniq", no_argument, NULL, UNIQ_ARG},
    {"group-by", required_argument, NULL, GROUP_BY_ARG},
    {"join", required_argument, NULL, JOIN_ARG},
    {NULL, 0, NULL, 0}
  };

/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recsel synopsis.
     no-wrap */
  printf (_("\
Usage: recsel [OPTION]... [-t TYPE] [-j FIELD] [-n INDEXES | -e RECORD_EXPR | -q STR | -m NUM] [-c | (-p|-P) FIELD_EXPR] [FILE]...\n"));

  /* TRANSLATORS: --help output, recsel arguments.
     no-wrap */
  fputs(_("\
Select and print rec data.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, recsel arguments.
     no-wrap */
  fputs (_("\
  -d, --include-descriptors           print record descriptors along with the matched\n\
                                        records.\n\
  -C, --collapse                      do not section the result in records with newlines.\n\
  -S, --sort=FIELD,...                sort the output by the specified fields.\n\
  -G, --group-by=FIELD,...            group records by the specified fields.\n\
  -U, --uniq                          remove duplicated fields in the output records.\n"),
         stdout);

#if defined REC_CRYPT_SUPPORT
  /* TRANSLATORS: --help output, encryption related options.
     no-wrap */
  fputs (_("\
  -s, --password=STR                  decrypt confidential fields with the given password.\n"),
         stdout);
#endif
  
  recutl_print_help_common ();

  puts ("");
  recutl_print_help_record_selection ();
  fputs (_("\
  -j, --join=FIELD                    perform an inner join using the specified field.\n"),
         stdout);

  puts ("");
  /* TRANSLATORS: --help output, recsel output options.
     no-wrap */
  fputs (_("\
Output options:\n\
  -p, --print=FIELDS                  comma-separated list of fields to print for each\n\
                                        matching record.\n\
  -P, --print-values=FIELDS           as -p, but print only the values of the selected\n\
                                        fields.\n\
  -R, --print-row=FIELDS              as -P, but separate the values with spaces instead\n\
                                        of newlines.\n\
  -c, --count                         print a count of the matching records instead of\n\
                                        the records themselves.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, recsel special options.
     no-wrap */
  fputs (_("\
Special options:\n\
      --print-sexps                   print the data in sexps instead of rec format.\n"),
         stdout);

  puts ("");
  recutl_print_help_footer ();
}

void
recsel_parse_args (int argc,
                   char **argv)
{
  char c;
  int ret;

  while ((ret = getopt_long (argc,
                             argv,
                             RECORD_SELECTION_SHORT_ARGS
                             ENCRYPTION_SHORT_ARGS
                             "S:Cdcp:P:R:UG:j:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
        COMMON_ARGS_CASES
        RECORD_SELECTION_ARGS_CASES
        case DESCRIPTOR_ARG:
        case 'd':
          {
            recsel_descriptors = true;
            break;
          }
        case PRINT_SEXPS_ARG:
          {
            recsel_write_mode = REC_WRITER_SEXP;
            break;
          }
        case UNIQ_ARG:
        case 'U':
          {
            recsel_uniq = true;
            break;
          }
#if defined REC_CRYPT_SUPPORT
        case PASSWORD_ARG:
        case 's':
          {
            if (recsel_password != NULL)
              {
                recutl_fatal (_("more than one password was specified\n"));
              }

            recsel_password = xstrdup (optarg);
            break;
          }
#endif
        case SORT_ARG:
        case 'S':
          {
            if (recutl_sort_by_fields)
              {
                recutl_fatal (_("only one field list can be specified as a sorting criteria.\n"));
              }

            /* Parse the field name.  */

            if (!rec_fex_check (optarg, REC_FEX_CSV))
              {
                recutl_fatal (_("invalid field names in -S.\n"));
              }

            recutl_sort_by_fields = rec_fex_new (optarg, REC_FEX_CSV);
            if (!recutl_sort_by_fields)
              {
                recutl_fatal (_("internal error creating fex.\n"));
              }

            break;
          }
        case JOIN_ARG:
        case 'j':
          {
            if (recsel_join)
              {
                recutl_fatal (_("only one field can be specified as join criteria.\n"));
              }

            if (!rec_field_name_p (optarg))
              {
                recutl_fatal (_("please specify a correct field name to -j|--join.\n"));
              }

            recsel_join = xstrdup (optarg);
            break;
          }
        case GROUP_BY_ARG:
        case 'G':
          {
            if (recsel_group_by_fields)
              {
                recutl_fatal (_("only one field list can be specified as a grouping criteria.\n"));
              }

            /* Parse the field name.  */
            if (!rec_fex_check (optarg, REC_FEX_CSV))
              {
                recutl_fatal (_("invalid field names in -G.\n"));
              }

            recsel_group_by_fields = rec_fex_new (optarg, REC_FEX_CSV);
            if (!recsel_group_by_fields)
              {
                recutl_fatal (_("internal error creating fex.\n"));
              }

            break;
          }
        case PRINT_ARG:
        case PRINT_VALUES_ARG:
        case PRINT_IN_A_ROW_ARG:
        case 'p':
        case 'P':
        case 'R':
          {
            if (recsel_count)
              {
                recutl_fatal (_("cannot specify -[pPR] and also -c.\n"));
              }

            if ((c == 'P') || (c == PRINT_VALUES_ARG))
              {
                recsel_write_mode = REC_WRITER_VALUES;
              }

            if ((c == 'R') || (c == PRINT_IN_A_ROW_ARG))
              {
                recsel_write_mode = REC_WRITER_VALUES_ROW;
              }

            recsel_fex_str = xstrdup (optarg);

            if (!rec_fex_check (recsel_fex_str, REC_FEX_SUBSCRIPTS))
              {
                recutl_fatal (_("invalid list of fields in -%c\n"), c);
              }

            /* Create the field expresion.  */
            recsel_fex = rec_fex_new (recsel_fex_str,
                                      REC_FEX_SUBSCRIPTS);
            if (!recsel_fex)
              {
                recutl_fatal (_("internal error creating the field expression.\n"));
              }

            /* Check that all the functions called in the fex exist.
               Otherwise raise an error.  */
            
            {
              size_t i = 0;
              for (i = 0; i < rec_fex_size (recsel_fex); i++)
                {
                  rec_fex_elem_t elem = rec_fex_get (recsel_fex, i);
                  const char *fname = rec_fex_elem_function_name (elem);

                  if (fname && !rec_aggregate_std_p (fname))
                    {
                      recutl_fatal (_("invalid aggregate function '%s'\n"), fname);
                    }
                }
            }

            break;
          }
        case COLLAPSE_ARG:
        case 'C':
          {
            recsel_collapse = true;
            break;
          }
        case COUNT_ARG:
        case 'c':
          {
            if (recsel_fex_str)
              {
                recutl_fatal (_("cannot specify -c and also -p.\n"));
                exit (EXIT_FAILURE);
              }

            recsel_count = true;
            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }

        }
    }

  /* Global checks on the parameters.  */

  if (!recutl_type && recsel_join)
    {
      recutl_fatal (_("joins can only be used when a named record set is selected.\n"));
    }
}

bool
recsel_process_data (rec_db_t db)
{
  int rset_size = 0;
  rec_rset_t rset = NULL;
  rec_writer_t writer = NULL;


#if defined REC_CRYPT_SUPPORT

  /* If recsel was called interactively and with an empty -s, was not
     used then prompt the user for it.  Otherwise use the password
     specified in the command line if any.  */

  if (!recsel_password
      && (recutl_type || (rec_db_size (db) == 1))
      && recutl_interactive ())
    {
      rec_rset_t rset;
      rec_fex_t confidential_fields;

      if (recutl_type)
        {
          rset = rec_db_get_rset_by_type (db, recutl_type);
        }
      else
        {
          rset = rec_db_get_rset (db, 0);
        }

      if (rset)
        {
          confidential_fields = rec_rset_confidential (rset);
          if (rec_fex_size (confidential_fields) > 0)
            {
              recsel_password = recutl_getpass (false);
            }
          
          rec_fex_destroy (confidential_fields);
        }
    }

  /* Note that the password must be at least one character long.  */

  if (recsel_password && (strlen (recsel_password) == 0))
    {
      free (recsel_password);
      recsel_password = NULL;
    }

#endif /* REC_CRYPT_SUPPORT */

  /* If the database contains more than one type of records and the
     user did'nt specify the recutl_type then ask the user to clarify
     the request.  */

  if (!recutl_type && (rec_db_size (db) > 1))
    {
      recutl_fatal (_("several record types found.  Please use -t to specify one.\n"));
    }


  /* Query the database using the criteria specified by the user in
     the command line.  */

  {
    int flags = 0;

    if (recutl_insensitive)
      {
        flags = flags | REC_F_ICASE;
      }

    if (recsel_descriptors)
      {
        flags = flags | REC_F_DESCRIPTOR;
      }

    if (recsel_uniq)
      {
        flags = flags | REC_F_UNIQ;
      }

    rset = rec_db_query (db,
                         recutl_type,
                         recsel_join,
                         recutl_index(),
                         recutl_sex,
                         recutl_quick_str,
                         recutl_random,
                         recsel_fex,
                         recsel_password,
                         recsel_group_by_fields,
                         recutl_sort_by_fields,
                         flags);
    if (!rset)
      recutl_out_of_memory ();
  }

  if (recsel_count)
    {
      /* Write the number of matching records.  */
      
      fprintf (stdout, "%d\n", rec_rset_num_records (rset));
    }
  else
    {
      /* Write the resulting record set to the standard output.  */

      writer = rec_writer_new (stdout);
      rec_writer_set_collapse (writer, recsel_collapse);
      rec_writer_set_skip_comments (writer, true);
      rec_writer_set_mode (writer, recsel_write_mode);
      rec_write_rset (writer, rset);
      rec_writer_destroy (writer);
    }

  rec_rset_destroy (rset);

  return true;
}

int
main (int argc, char *argv[])
{
  int res;
  rec_db_t db;

  res = 0;

  recutl_init ("recsel");

  /* Parse arguments.  */
  recsel_parse_args (argc, argv);

  /* Get the input data.  */
  db = recutl_build_db (argc, argv);
  if (!db)
    {
      res = 1;
      return res;
    }

  /* Process the data.  */
  if (!recsel_process_data (db))
    {
      res = 1;
    }

  rec_db_destroy (db);

  return res;
}

/* End of recsel.c */
