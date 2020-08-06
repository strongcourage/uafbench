/* -*- mode: C -*-
 *
 *       File:         recutl.c
 *       Date:         Thu Apr 22 17:30:48 2010
 *
 *       GNU recutils - Common code for the utilities.
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
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <closeout.h>
#include <xalloc.h>
#include <unistd.h>
#include <locale.h>
#include <gettext.h>
#define _(str) gettext (str)
#if defined REC_CRYPT_SUPPORT
#   include <gcrypt.h>
#endif
#include <progname.h>
#include <sys/stat.h>
#include <readline.h>
#include <regex.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <getpass.h>
#include <tmpdir.h>
#include <pathmax.h>
/* GNU/Hurd doesn't define PATH_MAX  */
#ifndef PATH_MAX
# define PATH_MAX 8192
#endif

#include <rec.h>
#include <recutl.h>
#include "read-file.h"

/*
 * Global variables.
 */

static bool    recutl_sort_p         = false;
static char   *recutl_order_rset     = NULL;
static char   *recutl_order_by_field = NULL;
static bool    recutl_interactive_p  = false;
static size_t *recutl_indexes        = NULL;
static size_t  recutl_indexes_size   = 0;

void recutl_print_help (void); /* Forward prototype.  */

void
recutl_init (char *util_name)
{
  set_program_name (xstrdup (util_name));

#if defined REC_CRYPT_SUPPORT
  /* Initialize libgcrypt */
  gcry_check_version (NULL);
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif

  /* Initialize librec */
  rec_init ();

  /* Even exiting has subtleties.  On exit, if any writes failed, change
     the exit status.  The /dev/full device on GNU/Linux can be used for
     testing; for instance, hello >/dev/full should exit unsuccessfully.
     This is implemented in the Gnulib module "closeout".  */
  atexit (close_stdout);

  /* i18n */
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  /* Detect whether the tool has been invoked interactively.  */
  
  recutl_interactive_p = isatty (fileno(stdin));

  /* Initially there are no indexes.  */

  recutl_reset_indexes ();
}

bool
recutl_interactive (void)
{
  return recutl_interactive_p;
}

void
recutl_print_help_footer (void)
{
  /* TRANSLATORS: --help output 5+ (reports)
     TRANSLATORS: the placeholder indicates the bug-reporting address
     for this application.  Please add _another line_ with the
     address for translation bugs.
     no-wrap */
  printf (_("\
Report bugs to: %s\n"), PACKAGE_BUGREPORT);
#ifdef PACKAGE_PACKAGER_BUG_REPORTS
  printf (_("Report %s bugs to: %s\n"), PACKAGE_PACKAGER,
          PACKAGE_PACKAGER_BUG_REPORTS);
#endif
#ifdef PACKAGE_URL
  printf (_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
#else
  printf (_("%s home page: <http://www.gnu.org/software/recutils/>\n"),
          PACKAGE_NAME, PACKAGE);
#endif
  fputs (_("General help using GNU software: <http://www.gnu.org/gethelp/>\n"),
         stdout);
}

void
recutl_print_help_common (void)
{
  /* TRANSLATORS: --help output, common arguments.
     no-wrap */
  fputs (_("\
      --help                          print a help message and exit.\n\
      --version                       show version and exit.\n"),
         stdout);
}

void
recutl_print_help_record_selection (void)
{
  /* TRANSLATORS: --help output, record selection arguments
     no-wrap */
  fputs (_("\
Record selection options:\n\
  -i, --case-insensitive              make strings case-insensitive in selection\n\
                                        expressions.\n\
  -t, --type=TYPE                     operate on records of the specified type only.\n\
  -e, --expression=RECORD_EXPR        selection expression.\n\
  -q, --quick=STR                     select records with fields containing a string.\n\
  -n, --number=NUM,...                select specific records by position, with ranges.\n\
  -m, --random=NUM                    select a given number of random records.\n"),
         stdout);
}

void
recutl_print_version (void)
{
  printf ("%s (GNU %s) %s\n",
          program_name,
          PACKAGE,
          VERSION);
  /* xgettext: no-wrap */
  puts ("");

  /* It is important to separate the year from the rest of the message,
     as done here, to avoid having to retranslate the message when a new
     year comes around.  */  
  printf (_("\
Copyright (C) %s Jose E. Marchesi.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n"), "2010-2019");

  puts (_("\
\n\
Written by Jose E. Marchesi."));
}

void
recutl_fatal (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fputs (program_name, stderr);
  fputs (_(": error: "), stderr);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  exit (EXIT_FAILURE);
}

void
recutl_out_of_memory (void)
{
  recutl_fatal (_("out of memory\n"));
}

void
recutl_error (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fputs (program_name, stderr);
  fputs (_(": error: "), stderr);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
}

void
recutl_warning (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fputs (program_name, stderr);
  fputs (_(": warning: "), stderr);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
}

bool
recutl_parse_db_from_file (FILE *in,
                           char *file_name,
                           rec_db_t db)
{
  bool res;
  rec_rset_t rset;
  rec_parser_t parser;

  res = true;

  parser = rec_parser_new (in, file_name);
  while (rec_parse_rset (parser, &rset))
    {
      char *rset_type;
      /* XXX: check for consistency!!!.  */
      rset_type = rec_rset_type (rset);
      if (rec_db_type_p (db, rset_type))
        {
          if (rset_type)
            recutl_fatal (_("duplicated record set '%s' from %s.\n"),
                          rset_type, file_name);
          else
            {
              /* Special case: the database already contains anonymous
                 records (with no type) and the record set to be
                 inserted also contains anonyous records.  In this
                 case we just append the records and comments in the
                 anonymous record set.  */

              rec_rset_t anon_rset = rec_db_get_rset_by_type (db, NULL);
              rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));
              rec_mset_elem_t elem;
              while (rec_mset_iterator_next (&iter, MSET_ANY, NULL, &elem))
                {
                  void *data = rec_mset_elem_dup_data (elem);
                  if (!data
                      || !rec_mset_append (rec_rset_mset (anon_rset),
                                           rec_mset_elem_type (elem),
                                           data,
                                           MSET_ANY))
                    return false;
                }
              rec_mset_iterator_free (&iter);
              return true;
            }
        }
          
      if (!rec_db_insert_rset (db, rset, rec_db_size (db)))
        {
          /* Error.  */
          res = false;
          break;
        }
    }

  if (rec_parser_error (parser))
    {
      /* Report parsing errors.  */
      rec_parser_perror (parser, "%s", file_name);
      res = false;
    }
  rec_parser_destroy (parser);

  return res;
}

rec_db_t
recutl_build_db (int argc, char **argv)
{
  rec_db_t db;
  char *file_name;
  FILE *in;

  db = rec_db_new ();
  if (!db)
    {
      return NULL;
    }

  /* Register the default functions in the database.  */

  

  /* Process the input files, if any.  Otherwise use the standard
     input to read the rec data.  */

  if (optind < argc)
    {
      while (optind < argc)
        {
          file_name = argv[optind++];
          if (!(in = fopen (file_name, "r")))
            {
              recutl_fatal (_("cannot read file %s\n"), file_name);
            }
          else
            {
              if (!recutl_parse_db_from_file (in, file_name, db))
                {
                  free (db);
                  db = NULL;
                }
              
              fclose (in);
            }
        }
    }
  else
    {
      if (!recutl_parse_db_from_file (stdin, "stdin", db))
        {
          free (db);
          db = NULL;
        }
    }

  return db;
}

rec_db_t
recutl_read_db_from_file (char *file_name)
{
  rec_db_t db;
  FILE *in;

  db = rec_db_new ();
  if (file_name)
    {
      in = fopen (file_name, "r");
      if (in == NULL)
        {
           return NULL;
        }
    }
  else
    {
      /* Process the standard input.  */
      file_name = "stdin";
      in = stdin;
    }

  if (!recutl_parse_db_from_file (in,
                                  file_name,
                                  db))
    {
      rec_db_destroy (db);
      db = NULL;
    }

  return db;
}

bool
recutl_file_is_writable (char *file_name)
{
  return !file_name || (euidaccess (file_name, W_OK) == 0);
}

void
recutl_write_db_to_file (rec_db_t db,
                         char *file_name)
{
  FILE *out;
  char tmp_file_name[PATH_MAX];
  rec_writer_t writer;
  int des;
  struct stat st1;
  int stat_result;

  if (!file_name)
    {
      out = stdout;
    }
  else
    {
      /* Record the original file attributes. */
      stat_result = stat (file_name, &st1);

      /* Create a temporary file with the results. */
      if ((path_search (tmp_file_name, PATH_MAX, NULL,
                        "rec", true) == -1)
          || ((des = mkstemp (tmp_file_name)) == -1))
        recutl_fatal (_("cannot create a unique name.\n"));

      out = fdopen (des, "w+");
    }

  writer = rec_writer_new (out);
  rec_write_db (writer, db);

  if (file_name)
    {
      fclose (out);
    }

  rec_db_destroy (db);

  if (file_name)
    {
      /* Rename the temporary file to file_name.  */
      if (rename (tmp_file_name, file_name) == -1)
        {
          remove (tmp_file_name);
          recutl_fatal (_("renaming file %s to %s\n"), tmp_file_name, file_name);
        }

      /* Restore the attributes of the original file. */
      if (stat_result != -1)
        {
          chmod (file_name, st1.st_mode);
        }
    }
}

char *
recutl_read_file (char *file_name)
{
  char *result;
  FILE *in;

  result = NULL;
  in = fopen (file_name, "r");
  if (in)
    {
      size_t file_size;
      result = fread_file (in, &file_size);
      fclose (in);
    }

  return result;
}

void
recutl_check_integrity (rec_db_t db,
                        bool verbose_p,
                        bool external_p)
{
  rec_buf_t errors_buf;
  char *errors_str;
  size_t errors_str_size;

  errors_buf = rec_buf_new (&errors_str, &errors_str_size);
  if (rec_int_check_db (db, true, external_p, errors_buf) > 0)
    {
      rec_buf_close (errors_buf);
      if (!verbose_p)
        {
          recutl_error (_("operation aborted due to integrity failures.\n"));
          recutl_error (_("use --verbose to get a detailed report.\n"));
        }
      else
        {
          fprintf (stderr, "%s", errors_str);
        }

      recutl_fatal (_("use --force to skip the integrity check.\n"));
    }
}

bool
recutl_yesno (char *prompt)
{
  bool res = false;
  char *line = NULL;

  while (1)
    {
      line = readline (prompt);
      if (line)
        {
          if (strcmp (line, "yes") == 0)
            {
              res = true;
              break;
            }
          else if (strcmp (line, "no") == 0)
            {
              res = false;
              break;
            }
        }

      printf ("Please answer 'yes' or 'no'.\n");
    }           

  return res;
}

#define INDEX_LIST_ENTRY_RE "[0-9]+(-[0-9]+)?"
#define INDEX_LIST_RE "(" INDEX_LIST_ENTRY_RE ",)*" INDEX_LIST_ENTRY_RE

bool
recutl_index_list_parse (const char *str)
{
  regex_t regexp;
  bool res = true;
  const char *p;
  long int number;
  char *end;
  size_t i;

  /* Initialize the list structure.   An pessimistic estimation of the
     number of indexes encoded in the string is used.  */

  free (recutl_indexes);
  recutl_indexes = xmalloc (sizeof (size_t) * (strlen (str) * 2 + 2));
  for (i = 0; i < (strlen (str) * 2 + 2); i++)
    {
      recutl_indexes[i] = REC_Q_NOINDEX;
    }
  
  /* Make sure the string is valid.  The code below relies on this
     fact.  */

  if (regcomp (&regexp, "^" INDEX_LIST_RE "$", REG_EXTENDED) != 0)
    {
      recutl_fatal (_("internal error: recutl_index_list_parse: error compiling regexp.\n"));
      return false;
    }

  if (regexec (&regexp, str, 0, NULL, 0) != 0)
    {
      regfree (&regexp);
      return false;
    }

  regfree (&regexp);

  /* Parse the string. */

  p = str;
  while (true)
    {
      /* Get the 'min' part of the entry.  */

      number = strtol (p, &end, 10);
      recutl_indexes[recutl_indexes_size] = (size_t) number;
      p = end;

      /* Get the 'max' part of the entry, if any.  */
      
      if (*p == '-')
        {
          p++;
          number = strtol (p, &end, 10);
          recutl_indexes[recutl_indexes_size+1] = (size_t) number;
          p = end;
        }

      recutl_indexes_size = recutl_indexes_size + 2;

      /* Exit or pass the separator.  */

      if (*p == '\0')
        {
          break;
        }
      else
        {
          p++;
        }
    }

  return res;
}

void
recutl_reset_indexes (void)
{
  free (recutl_indexes);
  recutl_indexes = NULL;
  recutl_indexes_size = 0;
}

size_t
recutl_num_indexes (void)
{
  return recutl_indexes_size;
}

size_t *
recutl_index (void)
{
  return recutl_indexes;
}

char *
recutl_getpass (bool asktwice)
{
  char *ret = NULL;

  char *pass = getpass (_("Password: "));
  if (pass)
    {
      ret = xstrdup (pass);
      if (asktwice)
        {
          pass = getpass (_("Password again: "));
          if (pass)
            {
              if (strcmp (ret, pass) != 0)
                {
                  recutl_fatal (_("the provided passwords don't match.\n"));
                  memset (ret, 0, strlen (ret));
                  memset (pass, 0, strlen (pass));
                }
            }
          else
            {
              memset (ret, 0, strlen (ret));
              free (ret);
              ret = NULL;
            }
        }
    }

  return ret;
}

/* End of recutl.c */
