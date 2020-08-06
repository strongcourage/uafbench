/* -*- mode: C -*-
 *
 *       File:         recfix.c
 *       Date:         Tue Apr 27 12:21:48 2010
 *
 *       GNU recutils - recfix
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
#include <stdlib.h>
#include <xalloc.h>
#include <gettext.h>
#define _(str) gettext (str)

#include <rec.h>
#include <recutl.h>

/* Forward prototypes.  */
static void recfix_parse_args (int argc, char **argv);
static bool recfix_check_database (rec_db_t db);

static int recfix_do_check (void);
static int recfix_do_sort (void);
#if defined REC_CRYPT_SUPPORT
static int recfix_do_crypt (void);
#endif
static int recfix_do_auto (void);

/*
 * Data types.
 */

/* recfix supports several operations, enumerated in the following
   type.  */

enum recfix_op
{
  RECFIX_OP_INVALID,
  RECFIX_OP_CHECK,
#if defined REC_CRYPT_SUPPORT
  RECFIX_OP_ENCRYPT,
  RECFIX_OP_DECRYPT,
#endif
  RECFIX_OP_SORT,
  RECFIX_OP_AUTO
};

/*
 * Global variables.
 */

bool  recfix_external = true;
char *recfix_file     = NULL;
int   recfix_op       = RECFIX_OP_INVALID;
char *recfix_password = NULL;
bool  recfix_force    = false;

/*
 * Command line options management.
 */

enum
{
  COMMON_ARGS,
  NO_EXTERNAL_ARG,
  FORCE_ARG,
  OP_SORT_ARG,
#if defined REC_CRYPT_SUPPORT
  PASSWORD_ARG,
  OP_ENCRYPT_ARG,
  OP_DECRYPT_ARG,
#endif
  OP_CHECK_ARG,
  OP_AUTO_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"no-external", no_argument, NULL, NO_EXTERNAL_ARG},
    {"force", no_argument, NULL, FORCE_ARG},
    {"check", no_argument, NULL, OP_CHECK_ARG},
    {"sort", no_argument, NULL, OP_SORT_ARG},
#if defined REC_CRYPT_SUPPORT
    {"password", required_argument, NULL, PASSWORD_ARG},
    {"encrypt", no_argument, NULL, OP_ENCRYPT_ARG},
    {"decrypt", no_argument, NULL, OP_DECRYPT_ARG},
#endif
    {"auto", no_argument, NULL, OP_AUTO_ARG},
    {NULL, 0, NULL, 0}
  };

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recfix synopsis.
     no-wrap */
  printf (_("\
Usage: recfix [OPTION]... [OPERATION] [OP_OPTION]... [FILE]\n"));

  /* TRANSLATORS: --help output, recfix short description.
     no-wrap */
  fputs (_("\
Check and fix rec files.\n"),
         stdout);

  puts ("");
  /* TRANSLATORS: --help output, recfix global arguments.
     no-wrap */
  fputs (_("\
      --no-external                   don't use external descriptors.\n\
      --force                         force the requested operation.\n"),
         stdout);

  recutl_print_help_common ();

  puts("");
  /* TRANSLATORS: --help output, recfix operations.
     no-wrap */
  fputs (_("\
Operations:\n\
      --check                         check integrity of the specified file.  Default.\n\
      --sort                          sort the records in the specified file.\n\
      --auto                          insert auto-generated fields in records missing them.\n"),
         stdout);

#if defined REC_CRYPT_SUPPORT
    /* TRANSLATORS: --help output, recfix operations related with encryption.
       no-wrap */
    fputs (_("\
      --encrypt                       encrypt confidential fields in the specified file.\n\
      --decrypt                       decrypt confidential fields in the specified file.\n"),
         stdout);

  puts("");
  /* TRANSLATORS: --help output, recfix encryption and decryption
     options.
     no-wrap */
  fputs (_("\
De/Encryption options:\n\
  -s, --password=PASSWORD             encrypt/decrypt with this password.\n"),
         stdout);
#endif /* REC_CRYPT_SUPPORT */

  puts("");
  /* TRANSLATORS: --help output, notes on recfix.
     no-wrap */
  fputs (_("\
If no FILE is specified then the command acts like a filter, getting\n\
the data from standard input and writing the result to standard output.\n"), stdout);

  puts("");
  recutl_print_help_footer ();
}

static void
recfix_parse_args (int argc,
                   char **argv)
{
  char c;
  int ret;

  while ((ret = getopt_long (argc,
                             argv,
                             ENCRYPTION_SHORT_ARGS,
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
        case NO_EXTERNAL_ARG:
          {
            recfix_external = false;
            break;
          }
        case FORCE_ARG:
          {
            recfix_force = true;
            break;
          }
#if defined REC_CRYPT_SUPPORT
        case 's':
        case PASSWORD_ARG:
          {
            if (recfix_op == RECFIX_OP_INVALID)
              {
                recutl_fatal (_("--password|-s must be used as an operation argument.\n"));
              }

            if ((recfix_op != RECFIX_OP_ENCRYPT)
                && (recfix_op != RECFIX_OP_DECRYPT))
              {
                recutl_fatal (_("the specified operation does not require a password.\n"));
              }

            if (recfix_password != NULL)
              {
                recutl_fatal (_("please specify just one password.\n"));
              }

            recfix_password = xstrdup (optarg);
            break;
          }
#endif /* REC_CRYPT_SUPPORT */
        case OP_CHECK_ARG:
          {
            if (recfix_op != RECFIX_OP_INVALID)
              {
                recutl_fatal (_("please specify just one operation.\n"));
              }

            recfix_op = RECFIX_OP_CHECK;
            break;
          }
        case OP_SORT_ARG:
          {
            if (recfix_op != RECFIX_OP_INVALID)
              {
                recutl_fatal (_("please specify just one operation.\n"));
              }

            recfix_op = RECFIX_OP_SORT;
            break;
          }
        case OP_AUTO_ARG:
          {
            if (recfix_op != RECFIX_OP_INVALID)
              {
                recutl_fatal (_("please specify just one operation.\n"));
              }

            recfix_op = RECFIX_OP_AUTO;
            break;
          }
#if defined REC_CRYPT_SUPPORT
        case OP_ENCRYPT_ARG:
          {
            if (recfix_op != RECFIX_OP_INVALID)
              {
                recutl_fatal (_("please specify just one operation.\n"));
              }

            recfix_op = RECFIX_OP_ENCRYPT;
            break;
          }
        case OP_DECRYPT_ARG:
          {
            if (recfix_op != RECFIX_OP_INVALID)
              {
                recutl_fatal (_("please specify just one operation.\n"));
              }

            recfix_op = RECFIX_OP_DECRYPT;
            break;
          }
#endif /* REC_CRYPT_SUPPORT */
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }

  /* The default operation is check, in case the user did not specify
     any in the command line.  */

  if (recfix_op == RECFIX_OP_INVALID)
    {
      recfix_op = RECFIX_OP_CHECK;
    }

#if defined REC_CRYPT_SUPPORT
  /* The encrypt and decrypt operations require the user to specify a
     password.  If no password was specified with -s and the program
     is running in a terminal, prompt the user to provide the
     password.  */

  if (((recfix_op == RECFIX_OP_ENCRYPT)
       || (recfix_op == RECFIX_OP_DECRYPT))
      && (recfix_password == NULL))
    {
      if (recutl_interactive ())
        {
          if (recfix_op == RECFIX_OP_ENCRYPT)
            {
              recfix_password = recutl_getpass (true);
            }
          else
            {
              recfix_password = recutl_getpass (false);
            }
        }

      if (!recfix_password || (strlen (recfix_password) == 0))
        {
          recutl_fatal ("please specify a password.\n");
        }
    }
#endif /* REC_CRYPT_SUPPORT */

  /* Read the name of the file to work on.  */
  if (optind < argc)
    {
      if ((argc - optind) != 1)
        {
          recutl_print_help ();
          exit (EXIT_FAILURE);
        }

      recfix_file = argv[optind++];
    }
}

static bool
recfix_check_database (rec_db_t db)
{
  bool ret;
  char *errors;
  size_t errors_size;
  rec_buf_t buf;

  buf = rec_buf_new (&errors, &errors_size);
  ret = (rec_int_check_db (db,
                           true,            /* Check descriptors.  */
                           recfix_external, /* Use external descriptors.  */
                           buf) == 0);
  rec_buf_close (buf);
  fprintf (stderr, "%s", errors);

  return ret;
}

static int
recfix_do_check ()
{
  rec_db_t db;

  /* Read the database from the specified file and check its
     integrity.  */

  db = recutl_read_db_from_file (recfix_file);
  if (!db)
    {
      return EXIT_FAILURE;
    }

  if (!recfix_check_database (db))
    {
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

static int
recfix_do_sort ()
{
  rec_db_t db     = NULL;
  size_t n_rset   = 0;
  rec_rset_t rset = NULL;

  /* Read the database from the specified file.  */

  db = recutl_read_db_from_file (recfix_file);
  if (!db)
    {
      return EXIT_FAILURE;
    }

  /* Sort all the record sets contained in the database.  */

  for (n_rset = 0; n_rset < rec_db_size (db); n_rset++)
    {
      rset = rec_db_get_rset (db, n_rset);
      if (!rec_rset_sort (rset, NULL))
        recutl_out_of_memory ();
    }
  
  if (!recfix_check_database (db))
    {
      return EXIT_FAILURE;
    }

  if (!recutl_file_is_writable (recfix_file))
    {
      recutl_error (_("file %s is not writable.\n"), recfix_file);
      return EXIT_FAILURE;
    }

  recutl_write_db_to_file (db, recfix_file);
  return EXIT_SUCCESS;
}

#if defined REC_CRYPT_SUPPORT

static int
recfix_do_crypt ()
{
  rec_db_t db;
  size_t n_rset;

  /* Read the database from the specified file. */

  db = recutl_read_db_from_file (recfix_file);
  if (!db)
    {
      return EXIT_FAILURE;
    }

  /* Encrypt/decrypt any unencrypted/encrypted field marked as
     "confidential" using the given password.  */

  for (n_rset = 0; n_rset < rec_db_size (db); n_rset++)
    {
      rec_mset_iterator_t iter;
      rec_fex_t confidential_fields;
      rec_record_t record;
      rec_rset_t rset =
        rec_db_get_rset (db, n_rset);

      /* Skip record sets not having any confidential fields.  */

      confidential_fields = rec_rset_confidential (rset);
      if (confidential_fields == NULL)
        {
          continue;
        }

      /* Process every record of the record set.  */

      iter = rec_mset_iterator (rec_rset_mset (rset));
      while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &record, NULL))
        {
          if (recfix_op == RECFIX_OP_ENCRYPT)
            {
              /* Encrypt any unencrypted confidential field in this
                 record.  */

              if (!rec_encrypt_record (rset, record, recfix_password)
                  && !recfix_force)
                {
                  recutl_error (_("the database contains already encrypted fields\n"));
                  recutl_fatal (_("please use --force or --decrypt\n"));
                }
            }
          else
            {
              /* Decrypt any encrypted confidential field in this
                 record.  */

              rec_decrypt_record (rset, record, recfix_password);
            }
        }

      rec_mset_iterator_free (&iter);
    }

  /* Write the modified database back to the file.  */

  recutl_write_db_to_file (db, recfix_file);

  return EXIT_SUCCESS;
}

#endif /* REC_CRYPT_SUPPORT */

static int
recfix_do_auto ()
{
  rec_db_t db    = NULL;
  size_t n_rset  = 0;

  /* Read the database from the especified file.  */

  db = recutl_read_db_from_file (recfix_file);
  if (!db)
    {
      return EXIT_FAILURE;
    }

  /* Add auto fields to any record in the database not having it, in
     record sets featuring auto fields.  */

  for (n_rset = 0; n_rset < rec_db_size (db); n_rset++)
    {
      rec_mset_iterator_t iter;
      rec_record_t record;
      rec_rset_t rset = rec_db_get_rset (db, n_rset);

      iter = rec_mset_iterator (rec_rset_mset (rset));
      while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void**) &record, NULL))
        {
          if (!rec_rset_add_auto_fields (rset, record))
            recutl_out_of_memory ();
        }

      rec_mset_iterator_free (&iter);
    }

  if (!recfix_check_database (db))
    {
      return EXIT_FAILURE;
    }

  recutl_write_db_to_file (db, recfix_file);
  return EXIT_SUCCESS;
}

int
main (int argc, char *argv[])
{
  int res     = EXIT_SUCCESS;

  recutl_init ("recfix");

  /* Parse arguments.  */

  recfix_parse_args (argc, argv);

  /* Execute the proper operation as specified in the recfix_op
     variable.  */

  switch (recfix_op)
    {
    case RECFIX_OP_CHECK:
      {
        res = recfix_do_check ();
        break;
      }
    case RECFIX_OP_SORT:
      {
        res = recfix_do_sort ();
        break;
      }
    case RECFIX_OP_AUTO:
      {
        res = recfix_do_auto ();
        break;
      }
#if defined REC_CRYPT_SUPPORT
    case RECFIX_OP_ENCRYPT:
    case RECFIX_OP_DECRYPT:
      {
        res = recfix_do_crypt ();
        break;
      }
#endif /* REC_CRYPT_SUPPORT */
    default:
      {
        /* This point shall not be reached.  */

        res = EXIT_FAILURE;
        recutl_fatal (_("unknown operation in recfix: please report this as a bug.\n"));
      }
    }

  return res;
}

/* End of recfix.c */
