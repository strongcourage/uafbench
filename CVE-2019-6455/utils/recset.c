/* -*- mode: C -*-
 *
 *       File:         recset.c
 *       Date:         Fri Apr  9 17:06:39 2010
 *
 *       GNU recutils - recset
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

/*
 * Forward prototypes.
 */

static void recset_parse_args (int argc, char **argv);
static void recset_process_actions (rec_db_t db);

/*
 * Global variables.
 */

char      *recutl_sex_str     = NULL;
rec_sex_t  recutl_sex         = NULL;
char      *recutl_quick_str   = NULL;
char      *recutl_fex_str     = NULL;
rec_fex_t  recutl_fex         = NULL;
char      *recutl_type        = NULL;
int        recset_action      = REC_SET_ACT_NONE;
char      *recset_value       = NULL;
char      *recset_new_field_name = NULL;
bool       recutl_insensitive = false;
char      *recset_file        = NULL;
bool       recset_force       = false;
bool       recset_verbose     = false;
bool       recset_external    = true;
bool       recset_descriptor_renamed = false;
size_t     recutl_random      = 0;

/*
 * Command line options management
 */

enum
  {
    COMMON_ARGS,
    RECORD_SELECTION_ARGS,
    FIELD_EXPR_ARG,
    ADD_ACTION_ARG,
    RENAME_ACTION_ARG,
    DELETE_ACTION_ARG,
    COMMENT_ACTION_ARG,
    SET_ACTION_ARG,
    SET_ADD_ACTION_ARG,
    FORCE_ARG,
    VERBOSE_ARG,
    NO_EXTERNAL_ARG
  };

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    RECORD_SELECTION_LONG_ARGS,
    {"fields", required_argument, NULL, FIELD_EXPR_ARG},
    {"add", required_argument, NULL, ADD_ACTION_ARG},
    {"rename", required_argument, NULL, RENAME_ACTION_ARG},
    {"delete", no_argument, NULL, DELETE_ACTION_ARG},
    {"comment", no_argument, NULL, COMMENT_ACTION_ARG},
    {"set", required_argument, NULL, SET_ACTION_ARG},
    {"set-add", required_argument, NULL, SET_ADD_ACTION_ARG},
    {"force", no_argument, NULL, FORCE_ARG},
    {"verbose", no_argument, NULL, VERBOSE_ARG},
    {"no-external", no_argument, NULL, NO_EXTERNAL_ARG},
    {NULL, 0, NULL, 0}
  };

/*
 * Functions.
 */


void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recset synopsis.
     no-wrap */
  printf (_("\
Usage: recset [OPTION]... [FILE]...\n"));

  /* TRANSLATORS: --help output, recset short description.
     no-wrap */
  fputs (_("\
Alter or delete fields in records.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, recset options.
     no-wrap */
  fputs (_("\
      --no-external                   don't use external descriptors.\n\
      --force                         alter the records even if violating record\n\
                                        restrictions.\n"), stdout);

  recutl_print_help_common ();

  puts ("");
  recutl_print_help_record_selection ();

  puts ("");
  /* TRANSLATORS: --help output, recset field selection options.
     no-wrap */
  fputs (_("\
Field selection options:\n\
  -f, --fields=FIELDS                 comma-separated list of field names with optional\n\
                                        subscripts.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, recset actions.
     no-wrap */
  fputs (_("\
Actions:\n\
  -s, --set=VALUE                     change the value of the selected fields.\n\
  -a, --add=VALUE                     add the selected fields with the given value.\n\
  -S, --set-add=VALUE                 change the value of the selected fields.  If they don't\n\
                                        exist then add a new field with that value.\n\
  -r, --rename=NAME                   rename the selected fields to a given name.  If an entire\n\
                                        record set is selected then the field is renamed in the\n\
                                        record descriptor as well.\n\
  -d, --delete                        delete the selected fields.\n\
  -c, --comment                       comment out the selected fields.\n"), stdout);

  puts ("");
  recutl_print_help_footer ();
}

/* This macro is used in recset_parse_args to avoid code repetition in
   the actions (set, add, delete, ...) switch cases..  */
#define CHECK_ACTION_PREREQ                     \
  do                                                            \
    {                                                           \
      if (!recutl_fex)                                          \
        {                                                       \
          recutl_fatal (_("please specify some field with -f.\n")); \
        }                                                       \
                                                                \
      if (recset_action != REC_SET_ACT_NONE)                     \
        {                                                       \
          recutl_fatal (_("please specify just one action.\n")); \
        }                                                       \
    }                                                           \
  while (0)

static void
recset_parse_args (int argc,
                   char **argv)
{
  int ret;
  char c;

  while ((ret = getopt_long (argc,
                             argv,
                             RECORD_SELECTION_SHORT_ARGS
                             "dct:s:S:a:f:r:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
          RECORD_SELECTION_ARGS_CASES
        case FORCE_ARG:
          {
            recset_force = true;
            break;
          }
        case VERBOSE_ARG:
          {
            recset_verbose = true;
            break;
          }
        case FIELD_EXPR_ARG:
        case 'f':
          {
            recutl_fex_str = xstrdup (optarg);
            if (!rec_fex_check (recutl_fex_str, REC_FEX_SUBSCRIPTS))
              {
                recutl_fatal (_("invalid field expression in -f.\n"));
              }

            /* Create the field expression.  */
            recutl_fex = rec_fex_new (recutl_fex_str,
                                      REC_FEX_SUBSCRIPTS);
            if (!recutl_fex)
              {
                recutl_fatal (_("creating the field expression.\n"));
              }

            /* Sort it.  */
            rec_fex_sort (recutl_fex);

            break;
          }
        case SET_ACTION_ARG:
        case 's':
          {
            CHECK_ACTION_PREREQ;
            recset_action = REC_SET_ACT_SET;
            recset_value = xstrdup (optarg);
            break;
          }
        case RENAME_ACTION_ARG:
        case 'r':
          {
            CHECK_ACTION_PREREQ;
            if (rec_fex_size (recutl_fex) != 1)
              {
                recutl_fatal (_("the rename operation requires just one field with an optional subscript.\n"));
              }

            recset_action = REC_SET_ACT_RENAME;
            recset_value = xstrdup (optarg);

            /* Validate the new name.  */
            recset_new_field_name = recset_value;
            if (!rec_field_name_p (recset_new_field_name))
              {
                recutl_fatal (_("invalid field name %s.\n"), recset_value);
              }
            
            break;
          }
        case ADD_ACTION_ARG:
        case 'a':
          {
            CHECK_ACTION_PREREQ;
            recset_action = REC_SET_ACT_ADD;
            recset_value = xstrdup (optarg);
            break;
          }
        case SET_ADD_ACTION_ARG:
        case 'S':
          {
            CHECK_ACTION_PREREQ;
            recset_action = REC_SET_ACT_SETADD;
            recset_value = xstrdup (optarg);
            break;
          }
        case DELETE_ACTION_ARG:
        case 'd':
          {
            CHECK_ACTION_PREREQ;
            recset_action = REC_SET_ACT_DELETE;
            break;
          }
        case COMMENT_ACTION_ARG:
        case 'c':
          {
            CHECK_ACTION_PREREQ;
            recset_action = REC_SET_ACT_COMMENT;
            break;
          }
        case NO_EXTERNAL_ARG:
          {
            recset_external = false;
            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
            break;
          }
        }
    }

  /* Read the name of the data source.  */
  if (optind < argc)
    {
      if ((argc - optind) != 1)
        {
          recutl_print_help ();
          exit (EXIT_FAILURE);
        }

      recset_file = argv[optind++];
    }

}

static void
recset_process_actions (rec_db_t db)
{
  int flags = 0;

  if (recutl_insensitive)
    {
      flags = flags | REC_F_ICASE;
    }

  if (!rec_db_set (db,
                   recutl_type,
                   recutl_index (),
                   recutl_sex,
                   recutl_quick_str,
                   recutl_random,
                   recutl_fex,
                   recset_action,
                   recset_value,
                   flags))
    recutl_out_of_memory ();
  
  /* Check the integrity of the resulting database.  */

  if (!recset_force && db)
    {
      recutl_check_integrity (db, recset_verbose, recset_external);
    }
}

int
main (int argc, char *argv[])
{
  rec_db_t db;
  
  recutl_init ("recset");

  /* Parse arguments.  */
  recset_parse_args (argc, argv);

  db = recutl_read_db_from_file (recset_file);
  if (!db)
    {
      recutl_fatal (_("cannot read file %s\n"), recset_file);
    }

  recset_process_actions (db);

  if (!recutl_file_is_writable (recset_file))
    {
      recutl_error (_("file %s is not writable.\n"), recset_file);
      return EXIT_FAILURE;
    }
  recutl_write_db_to_file (db, recset_file);

  return EXIT_SUCCESS;
}

/* End of recset.c */
