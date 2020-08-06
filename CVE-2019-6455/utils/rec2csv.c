/* -*- mode: C -*-
 *
 *       File:         rec2csv.c
 *       Date:         Mon Jan 31 22:12:29 2011
 *
 *       GNU recutils - rec to csv converter.
 *
 */

/* Copyright (C) 2011-2019 Jose E. Marchesi */

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

#include <csv.h>
#include <rec.h>
#include <recutl.h>

/* Forward declarations.  */
static void rec2csv_parse_args (int argc, char **argv);
static bool rec2csv_process_data (rec_db_t db);
static rec_fex_t rec2csv_determine_fields (rec_rset_t rset);
static void rec2csv_generate_csv (rec_rset_t rset, rec_fex_t fex);

/*
 * Types
 */

/*
 * Global variables
 */

char             *rec2csv_record_type    = NULL;
rec_fex_t         rec2csv_sort_by_fields = NULL;
char              rec2csv_delim          = ',';

/*
 * Command line options management
 */

enum
  {
    COMMON_ARGS,
    RECORD_TYPE_ARG,
    SORT_ARG
  };

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"type", required_argument, NULL, RECORD_TYPE_ARG},
    {"sort", required_argument, NULL, SORT_ARG},
    {NULL, 0, NULL, 0}
  };


/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, rec2csv synopsis.
     no-wrap */
  printf (_("\
Usage: rec2csv [OPTIONS]... [REC_FILE]\n"));

  /* TRANSLATORS: --help output, rec2csv short description.
     no-wrap */
  fputs (_("\
Convert rec data into csv data.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, rec2csv options.
     no-wrap */
  fputs (_("\
  -d, --delim=char                    sets the deliminator (default ',')\n\
  -t, --type=TYPE                     record set to convert to csv; if this parameter\n\
                                        is omitted then the default record set is used\n\
  -S, --sort=FIELDS                   sort the output by the specified fields.\n"),
         stdout);

  recutl_print_help_common ();
  puts ("");
  recutl_print_help_footer ();
}

static void
rec2csv_parse_args (int argc,
                    char **argv)
{
  int ret;
  char c;

  while ((ret = getopt_long (argc,
                             argv,
                             "t:S:d:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
        case RECORD_TYPE_ARG:
        case 'd':
          {
            rec2csv_delim = optarg[0];
            break;
          }
        case 't':
          {
            rec2csv_record_type = xstrdup (optarg);
            break;
          }
        case SORT_ARG:
        case 'S':
          {
            if (rec2csv_sort_by_fields)
              {
                recutl_fatal (_("only one list of fields can be specified as a sorting criteria.\n"));
              }

            /* Parse the field name.  */

            if (!rec_fex_check (optarg, REC_FEX_CSV))
              {
                recutl_fatal (_("invalid field name list in -S.\n"));
              }

            rec2csv_sort_by_fields = rec_fex_new (optarg, REC_FEX_CSV);
            if (!rec2csv_sort_by_fields)
              {
                recutl_fatal (_("internal error creating fex.\n"));
              }

            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }
}

static void
rec2csv_generate_csv (rec_rset_t rset,
                      rec_fex_t fex)
{
  rec_mset_iterator_t iter;
  rec_fex_elem_t fex_elem;
  rec_record_t record;
  rec_field_t field;
  char *field_name;
  char *tmp;
  size_t i;

  /* Generate the row with headers.  */
  for (i = 0; i < rec_fex_size (fex); i++)
    {
      if (i != 0)
        {
          putc (rec2csv_delim, stdout);
        }

      fex_elem = rec_fex_get (fex, i);
      field_name = xstrdup (rec_fex_elem_field_name (fex_elem));

      /* The header is FNAME or FNAME_N where N is the index starting
         at 1.  Note that we shall remove the trailing ':', if any. */

      if (field_name[strlen(field_name)-1] == ':')
        {
          field_name[strlen(field_name)-1] = '\0';
        }


      if (rec_fex_elem_min (fex_elem) != 0)
        {
          if (asprintf (&tmp, "%s_%d",
                        field_name,
                        rec_fex_elem_min (fex_elem) + 1) == -1)
            recutl_out_of_memory ();
        }
      else
        {
          if (asprintf (&tmp, "%s", field_name) == -1)
            recutl_out_of_memory ();
        }

      csv_fwrite (stdout, tmp, strlen(tmp));
      free (field_name);
      free (tmp);
    }

  putc ('\n', stdout);

  /* Generate the data rows.  */

  iter = rec_mset_iterator (rec_rset_mset (rset));
  while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void**) &record, NULL))
    {
      for (i = 0; i < rec_fex_size (fex); i++)
        {
          if (i != 0)
            {
              putc (rec2csv_delim, stdout);
            }

          fex_elem = rec_fex_get (fex, i);
          field = rec_record_get_field_by_name (record,
                                                rec_fex_elem_field_name (fex_elem),
                                                rec_fex_elem_min (fex_elem));
          if (field)
            {
              csv_fwrite (stdout,
                          rec_field_value (field),
                          strlen (rec_field_value (field)));
            }
        }

      putc ('\n', stdout);
    }

  rec_mset_iterator_free (&iter);
}

static rec_fex_t
rec2csv_determine_fields (rec_rset_t rset)
{
  rec_fex_t fields;
  rec_mset_iterator_t iter_rset;
  rec_mset_iterator_t iter_record;
  rec_record_t record;
  rec_field_t field;
  int field_index;
  
  fields = rec_fex_new (NULL, REC_FEX_SIMPLE);

  iter_rset = rec_mset_iterator (rec_rset_mset (rset));
  while (rec_mset_iterator_next (&iter_rset, MSET_RECORD, (const void **) &record, NULL))
    {
      iter_record = rec_mset_iterator (rec_record_mset (record));
      while (rec_mset_iterator_next (&iter_record, MSET_FIELD, (const void **) &field, NULL))
        {
          field_index = rec_record_get_field_index_by_name (record, field);
          
          if (!rec_fex_member_p (fields,
                                 rec_field_name (field),
                                 field_index, field_index))
            {
              rec_fex_append (fields,
                              rec_field_name (field),
                              field_index, field_index);
            }
        }

      rec_mset_iterator_free (&iter_record);
    }

  rec_mset_iterator_free (&iter_rset);

  return fields;
}

static bool
rec2csv_process_data (rec_db_t db)
{
  bool ret;
  rec_fex_t row_fields;
  size_t i;
  rec_rset_t rset;

  ret = true;

  for (i = 0; i < rec_db_size (db); i++)
    {
      rset = rec_db_get_rset (db, i);
      if (((rec2csv_record_type)
           && rec_rset_type (rset)
           && (strcmp (rec_rset_type (rset),
                       rec2csv_record_type) == 0))
          || (!rec2csv_record_type
              && (!rec_rset_type (rset) ||
                  (rec_db_size (db) == 1))))
        {
          /* Process this record set.  */

          if (!rec_rset_sort (rset, rec2csv_sort_by_fields))
            recutl_out_of_memory ();

          /* Build the fields that will appear in the row. */
          row_fields = rec2csv_determine_fields (rset);
  
          /* Generate the csv data.  */
          rec2csv_generate_csv (rset, row_fields);

          /* Cleanup.  */
          rec_fex_destroy (row_fields);
        }
    }

  return ret;
}

int
main (int argc, char *argv[])
{
  int res;
  rec_db_t db;

  res = 0;

  recutl_init ("rec2csv");

  /* Parse arguments.  */
  rec2csv_parse_args (argc, argv);

  /* Get the input data.  */
  db = recutl_build_db (argc, argv);
  if (!db)
    {
      res = 1;
    }
  else
    /* Process the data.  */
    if (!rec2csv_process_data (db))
      {
        res = 1;
      }

  rec_db_destroy (db);
  
  return res;
}

/* End of rec2csv.c */
