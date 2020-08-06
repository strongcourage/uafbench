/* -*- mode: C -*-
 *
 *       File:         csv2rec.c
 *       Date:         Fri Aug 20 16:35:25 2010
 *
 *       GNU recutils - csv to rec converter.
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

#include <csv.h>
#include <rec.h>
#include <recutl.h>

/* Forward declarations.  */
static void parse_args (int argc, char **argv);
static rec_db_t process_csv (void);
static int is_space (unsigned char c);
static int is_term (unsigned char c);
static void field_cb (void *s, size_t len, void *data);
static void record_cb (int c, void *data);

/*
 * Types
 */

struct csv2rec_ctx
{
  rec_db_t db;
  rec_rset_t rset;
  rec_record_t record;

  size_t num_fields;
  size_t lineno;

#define ALLOC_FIELDS 256
  bool header_p;
  size_t num_field_names;
  char **field_names;
};

/*
 * Global variables
 */

char *csv2rec_record_type = NULL;
char *csv2rec_csv_file = NULL;
bool csv2rec_strict = false;
bool csv2rec_omit_empty = false;

/*
 * Command line options management
 */

enum
  {
    COMMON_ARGS,
    RECORD_TYPE_ARG,
    STRICT_ARG,
    OMIT_EMPTY_ARG
  };

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"type", required_argument, NULL, RECORD_TYPE_ARG},
    {"strict", no_argument, NULL, STRICT_ARG},
    {"omit-empty", no_argument, NULL, OMIT_EMPTY_ARG},
    {NULL, 0, NULL, 0}
  };

/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, csv2rec synopsis.
     no-wrap */
  printf (_("\
Usage: csv2rec [OPTIONS]... [CSV_FILE]\n"));

  /* TRANSLATORS: --help output, csv2rec short description.
     no-wrap */
  fputs (_("\
Convert csv data into rec data.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, csv2rec options.
     no-wrap */
  fputs (_("\
  -t, --type=TYPE                     type name for the converted records; if this\n\
                                        parameter is ommited then no type is used.\n\
  -s, --strict                        be strict parsing the csv file.\n\
  -e, --omit-empty                    omit empty fields.\n"), stdout);

  recutl_print_help_common ();
  puts ("");
  recutl_print_help_footer ();
}

static void
parse_args (int argc,
            char **argv)
{
  int ret;
  char c;

  while ((ret = getopt_long (argc,
                             argv,
                             "t:se",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
        case RECORD_TYPE_ARG:
        case 't':
          {
            csv2rec_record_type = xstrdup (optarg);
            break;
          }
        case STRICT_ARG:
        case 's':
          {
            csv2rec_strict = true;
            break;
          }
        case OMIT_EMPTY_ARG:
        case 'e':
          {
            csv2rec_omit_empty = true;
            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }

  /* Read the name of the csv file, if any.  */
  if (optind < argc)
    {
      if ((argc - optind) != 1)
        {
          recutl_print_help ();
          exit (EXIT_FAILURE);
        }

      csv2rec_csv_file = argv[optind++];
    }
}

static int
is_space (unsigned char c)
{
  return (c == CSV_SPACE) || (c == CSV_TAB);
}

static int
is_term (unsigned char c)
{
  return (c == CSV_CR) || (c == CSV_LF);
}

void
field_cb (void *s, size_t len, void *data)
{
  char *str;
  char *field_name;
  rec_field_t field;
  struct csv2rec_ctx *ctx;
  size_t i;

  ctx = (struct csv2rec_ctx *) data;
  str = xmalloc (len + 1);
  memcpy (str, s, len);
  str[len] = '\0';

  if (ctx->header_p)
    {
      /* Add a new field name to ctx.field_names.  */

      if ((ctx->num_field_names % ALLOC_FIELDS) == 0)
        ctx->field_names =
          realloc (ctx->field_names, ((ctx->num_field_names / ALLOC_FIELDS) + 1) * (sizeof(char *) * ALLOC_FIELDS));

      /* Normalize the name: spaces and tabs are turned into dashes
         '_'.  */
      for (i = 0; i < strlen (str); i++)
        {
          if ((str[i] == ' ') || (str[i] == '\t'))
            {
              str[i] = '_';
            }
        }

      /* Verify that it is a valid field name.  */
      field_name = str;
      if (!rec_field_name_p (field_name))
        {
          recutl_fatal (_("invalid field name '%s' in header\n"),
                        str);
        }
      ctx->field_names[ctx->num_field_names++] = str;
    }
  else
    {
      /* Create a new field and insert it in the current record.  */

      if (!ctx->record)
        {
          /* Create a new record.  */
          ctx->record = rec_record_new ();
          if (!ctx->record)
            recutl_out_of_memory ();
        }
      
      if (!csv2rec_omit_empty || (strlen(str) > 0))
        {
          if (ctx->num_fields > ctx->num_field_names)
            {
              char *source = csv2rec_csv_file;

              if (!source)
                {
                  source = "stdin";
                }

              fprintf (stderr,
                       _("%s: %lu: this line contains %lu fields, but %lu header fields were read\n"),
                       source,
                       ctx->lineno, ctx->num_field_names, ctx->num_fields);
              exit (EXIT_FAILURE);
            }
          field = rec_field_new (ctx->field_names[ctx->num_fields], str);
          rec_mset_append (rec_record_mset (ctx->record), MSET_FIELD, (void *) field, MSET_ANY);
        }

      ctx->num_fields++;
    }
}

void
record_cb (int c, void *data)
{
  struct csv2rec_ctx *ctx;
  ctx = (struct csv2rec_ctx *) data;

  ctx->lineno++;

  if (ctx->header_p)
    {
      ctx->header_p = false;
    }
  else
    {
      if (!ctx->rset)
        {
          /* Create a new record set.  */
          ctx->rset = rec_rset_new ();
          if (!ctx->rset)
            recutl_out_of_memory ();

          /* Add a type, if needed.  */
          if (csv2rec_record_type)
            {
              rec_rset_set_type (ctx->rset, csv2rec_record_type);
            }
          
          /* Add it to the database.  */
          if (!ctx->db)
            {
              ctx->db = rec_db_new ();
              if (!ctx->db)
                recutl_out_of_memory ();
            }
          rec_db_insert_rset (ctx->db, ctx->rset, rec_db_size (ctx->db));
        }
      
      /* Add the current record to the record set.  */
      rec_mset_append (rec_rset_mset (ctx->rset), MSET_RECORD, (void *) ctx->record, MSET_ANY);
      ctx->record = NULL;
      
      /* Reset the field counter.  */
      ctx->num_fields = 0;
    }
}

static rec_db_t
process_csv (void)
{
  struct csv2rec_ctx ctx;
  FILE *in;
  struct csv_parser p;
  unsigned char options = 0;
  char buf[1024];
  size_t bytes_read = 0;

  /* Initialize the data in the context.  */
  ctx.db = NULL;
  ctx.rset = NULL;
  ctx.record = NULL;
  ctx.header_p = true;
  ctx.field_names = NULL;
  ctx.num_field_names = 0;
  ctx.num_fields = 0;
  ctx.lineno = 0;

  /* Set the files to read/write from/to.

     If a filename was specified, read the csv file from there.
     Otherwise use the standard input.  The output is written to the
     standard output in any case.  */
  if (csv2rec_csv_file)
    {
      if (!(in = fopen (csv2rec_csv_file, "r")))
        {
          recutl_fatal (_("cannot read file %s\n"), csv2rec_csv_file);
        }
    }
  else
    {
      in = stdin;
    }

  /* Initialize the csv library.  */
  if (csv_init (&p, options) != 0)
    {
      recutl_fatal (_("failed to initialize csv parser\n"));
    }

  /* Set some properties of the parser.  */
  if (csv2rec_strict)
    {
      options |= CSV_STRICT;
      csv_set_opts (&p, options);
    }

  csv_set_space_func (&p, is_space);
  csv_set_term_func  (&p, is_term);

  /* Parse the input file in chunks of data.  */
  while ((bytes_read = fread (buf, 1, 1024, in)) > 0)
    {
      if (csv_parse (&p, buf, bytes_read, field_cb, record_cb, &ctx) != bytes_read)
        {
          recutl_fatal (_("error while parsing CSV file: %s\n"),
                        csv_strerror (csv_error (&p)));
        }

    }
  
  return ctx.db;
}

int
main (int argc, char *argv[])
{
  int ret;
  rec_db_t db;
  rec_writer_t writer;

  recutl_init ("csv2rec");

  parse_args (argc, argv);
  db = process_csv ();
  ret = EXIT_SUCCESS;

  if (db)
    {
      writer = rec_writer_new (stdout);
      rec_write_db (writer, db);

      rec_writer_destroy (writer);
      rec_db_destroy (db);
    }
  else
    {
      ret = EXIT_FAILURE;
    }

  return ret;
}

/* End of csv2rec.c */
