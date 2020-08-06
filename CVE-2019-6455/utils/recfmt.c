/* -*- mode: C -*-
 *
 *       File:         recfmt.c
 *       Date:         Wed Dec 22 18:20:20 2010
 *
 *       GNU recutils - recfmt
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

#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <xalloc.h>
#include <regex.h>
#include <gettext.h>
#define _(str) gettext (str)

#include <rec.h>
#include <recutl.h>

/* Forward prototypes.  */
void recfmt_parse_args (int argc, char **argv);
bool recfmt_process_data (rec_db_t db);
void recfmt_process_db (rec_db_t db, char *template);
char *recfmt_apply_template (rec_record_t record, char *template);
char *recfmt_get_subst (rec_record_t record, char *str);

/*
 * Global variables
 */

char *recfmt_template = NULL;

/*
 * Command line options management.
 */

enum
{
  COMMON_ARGS,
  FILE_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"file", required_argument, NULL, FILE_ARG},
    {NULL, 0, NULL, 0}
  };

/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recfmt synopsis.
     no-wrap */
  printf (_("\
Usage: recfmt [OPTION]... [TEMPLATE]\n"));

  /* TRANSLATORS: --help output, recfmt arguments.
     no-wrap */
  fputs(_("\
Apply a template to records read from standard input.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, recfmt arguments.
     no-wrap */
  fputs(_("\
  -f, --file=FILENAME                 read the template to apply from a file.\n"),
        stdout);

  recutl_print_help_common ();
  puts ("");
  recutl_print_help_footer ();
}

void
recfmt_parse_args (int argc,
                   char **argv)
{
  char c;
  int ret;

  while ((ret = getopt_long (argc,
                             argv,
                             "f:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
        case FILE_ARG:
        case 'f':
          {
            /* Read the template from the specified file and store it
               in recfmt_template.  */
            recfmt_template = recutl_read_file (optarg);
            if (!recfmt_template)
              {
                recutl_fatal (_("can't open file %s for reading.\n"),
                              optarg);
              }

            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }

  /* See if the template is specified in the command line.  */
  if (optind < argc)
    {
      if (recfmt_template)
        {
          recutl_fatal (_("don't specify a template in the command line and -f at the same time.\n"));
        }

      if ((argc - optind) != 1)
        {
          recutl_print_help ();
          exit (EXIT_FAILURE);
        }

      recfmt_template = xstrdup (argv[optind++]);
    }
}

char *
recfmt_get_subst (rec_record_t record,
                  char *str)
{
  char *res;
  rec_sex_t sex;

  sex = rec_sex_new (false);
  if (!rec_sex_compile (sex, str))
    {
      recutl_fatal (_("invalid expression in a template slot.\n"));
    }

  res = rec_sex_eval_str (sex, record);
  if (!res)
    {
      recutl_fatal (_("error evaluating expression in a template slot.\n"));
    }

  rec_sex_destroy (sex);

  return res;
}

char *
recfmt_apply_template (rec_record_t record,
                       char *template)
{
  rec_buf_t result_buf;
  char *result;
  char *tmp;
  size_t tmp_size;
  size_t result_size;
  char *p;
  regex_t regexp;
  regmatch_t matches;
  char *subst_str;

  /* Replace occurrences of:

     {{Name[N]}}

     where Name[N] is the name of a field with an optional subscript.
     If the subscript is not present then it is assumed to be 0.  If
     the contents between {{...}} are not a field name then replace the
     slot with the empty string.
  */

  if (regcomp (&regexp, "\\{\\{" "[^}]*" "\\}\\}", REG_EXTENDED) != 0)
    {
      recutl_fatal (_("recfmt_apply_template: error compiling regexp. Please report this.\n"));
    }

  result_buf = rec_buf_new (&result, &result_size);
  p = template;
  while (*p
         && (regexec (&regexp, p, 1, &matches, 0) == 0)
         && (matches.rm_so != -1))
    {
      /* Add the prolog, if any.  */
      if (matches.rm_so > 0)
        {
          tmp = xmalloc (matches.rm_so + 1);
          memcpy (tmp, p, matches.rm_so);
          tmp[matches.rm_so] = '\0';
          rec_buf_puts (tmp, result_buf);
          free (tmp);
        }

      /* Get the match.  */
      tmp_size = matches.rm_eo - matches.rm_so - 4;
      tmp = xmalloc (tmp_size + 1);
      memcpy (tmp, p + matches.rm_so + 2, tmp_size);
      tmp[tmp_size] = '\0';

      /* Advance p. */
      p = p + matches.rm_eo;

      /* Get the substitution text and append it to the result.  */
      subst_str = recfmt_get_subst (record, tmp);
      if (subst_str)
        {
          rec_buf_puts (subst_str, result_buf);
          free (subst_str);
        }
      free (tmp);
    }

  /* Add the epilog, if any.  */
  if (*p)
    {
      rec_buf_puts (p, result_buf);
    }
  
  rec_buf_close (result_buf);

  return result;
}

void
recfmt_process_db (rec_db_t db, char *template)
{
  size_t n_rset;
  rec_rset_t rset;
  rec_record_t record;
  char *result;
  rec_mset_iterator_t iter;

  /* Iterate on all the record sets.  */
  for (n_rset = 0; n_rset < rec_db_size (db); n_rset++)
    {
      rset = rec_db_get_rset (db, n_rset);

      /* Iterate on all the records.  */

      iter = rec_mset_iterator (rec_rset_mset (rset));
      while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void**) &record, NULL))
        {
          /* Apply the template to this record.  */
          result = recfmt_apply_template (record, template);
          if (result && (*result != '\0'))
            {
              printf ("%s", result);
              free (result);
            }
        }

      rec_mset_iterator_free (&iter);
    }
}

int
main (int argc, char *argv[])
{
  rec_db_t db;

  recutl_init ("recfmt");

  recfmt_parse_args (argc, argv);

  db = recutl_read_db_from_file (NULL);
  if (db && recfmt_template)
    {
      recfmt_process_db (db, recfmt_template);
    }

  return EXIT_SUCCESS;
}

/* End of recfmt.c */
