/* -*- mode: C -*-
 *
 *       File:         recutl.h
 *       Date:         Thu Apr 22 17:29:52 2010
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

#ifndef RECUTL_H
#define RECUTL_H

#include <progname.h>

/*
 * Common arguments.
 */

#define COMMON_ARGS                             \
  HELP_ARG,                                     \
  VERSION_ARG

#define COMMON_LONG_ARGS                        \
  {"help", no_argument, NULL, HELP_ARG},        \
  {"version", no_argument, NULL, VERSION_ARG}

#define COMMON_ARGS_CASES                          \
  case HELP_ARG:                                   \
    {                                              \
      recutl_print_help ();                        \
      exit (EXIT_SUCCESS);                         \
      break;                                       \
    }                                              \
  case VERSION_ARG:                                \
    {                                              \
      recutl_print_version ();                     \
      exit (EXIT_SUCCESS);                         \
      break;                                       \
    }

/*
 * Record selection arguments.
 */

#define RECORD_SELECTION_ARGS                   \
  TYPE_ARG,                                     \
  EXPRESSION_ARG,                               \
  QUICK_ARG,                                    \
  NUM_ARG,                                      \
  INSENSITIVE_ARG,                              \
  RANDOM_ARG

#define RECORD_SELECTION_LONG_ARGS                                     \
   {"type", required_argument, NULL, TYPE_ARG},                        \
   {"expression", required_argument, NULL, EXPRESSION_ARG},            \
   {"quick", required_argument, NULL, QUICK_ARG},                      \
   {"number", required_argument, NULL, NUM_ARG},                       \
   {"case-insensitive", no_argument, NULL, INSENSITIVE_ARG},           \
   {"random", required_argument, NULL, RANDOM_ARG}

#define RECORD_SELECTION_SHORT_ARGS             \
   "it:e:n:q:m:"

#define RECORD_SELECTION_ARGS_CASES                            \
    case TYPE_ARG:                                             \
    case 't':                                                  \
      {                                                        \
        recutl_type = xstrdup (optarg);                        \
        if (!rec_field_name_p (recutl_type))                   \
          {                                                    \
             recutl_fatal ("invalid record type %s\n",         \
                           recutl_type);                       \
          }                                                    \
        break;                                                 \
      }                                                        \
    case EXPRESSION_ARG:                                       \
    case 'e':                                                  \
      {                                                        \
        if (recutl_num_indexes() != 0)                         \
          {                                                    \
            recutl_fatal (_("cannot specify -e and also -n\n"));\
          }                                                    \
                                                               \
        if (recutl_quick_str)                                  \
          {                                                    \
            recutl_fatal (_("cannot specify -e and also -q\n"));\
          }                                                    \
                                                               \
         recutl_sex_str = xstrdup (optarg);                    \
                                                               \
         /* Compile the search expression.  */                 \
         if (recutl_sex_str)                                   \
          {                                                    \
            recutl_sex = rec_sex_new (recutl_insensitive);     \
            if (!rec_sex_compile (recutl_sex, recutl_sex_str)) \
             {                                                 \
               recutl_fatal(_("invalid selection expression\n"));       \
             }                                                 \
          }                                                    \
                                                               \
         break;                                                \
      }                                                        \
      case NUM_ARG:                                            \
      case 'n':                                                \
      {                                                        \
                                                               \
         if (recutl_sex)                                       \
          {                                                    \
             recutl_fatal(_("cannot specify -n and also -e\n"));\
          }                                                    \
                                                               \
        if (recutl_quick_str)                                  \
          {                                                    \
             recutl_fatal(_("cannot specify -n and also -q\n"));\
          }                                                    \
                                                               \
        if (recutl_num_indexes() != 0)                         \
          {                                                    \
             recutl_fatal ("please specify just one -n\n");    \
          }                                                    \
                                                               \
        if (!recutl_index_list_parse (optarg))                 \
          {                                                    \
            recutl_fatal (_("invalid list of indexes in -n\n")); \
          }                                                    \
                                                               \
          break;                                               \
      }                                                        \
      case RANDOM_ARG:                                         \
      case 'm':                                                \
      {                                                        \
        if (recutl_sex)                                        \
          {                                                    \
             recutl_fatal (_("cannot specify -m and also -e\n"));\
          }                                                    \
                                                               \
        if (recutl_quick_str)                                  \
          {                                                    \
             recutl_fatal (_("cannot specify -m and also -q\n"));\
          }                                                    \
                                                               \
        if (recutl_num_indexes() != 0)                         \
          {                                                    \
             recutl_fatal (_("cannot specify -m and also -n\n"));\
          }                                                    \
                                                               \
        if (recutl_random)                                     \
          {                                                    \
             recutl_fatal ("please specify just one -m\n");    \
          }                                                    \
                                                               \
        {                                                      \
          char *end;                                           \
          long int li = strtol (optarg, &end, 0);              \
          if (*end != '\0')                                    \
            {                                                  \
              recutl_fatal ("invalid number in -m\n");         \
            }                                                  \
                                                               \
          recutl_random = li;                                  \
        }                                                      \
                                                               \
        break;                                                 \
      }                                                        \
      case QUICK_ARG:                                          \
      case 'q':                                                \
      {                                                        \
         if (recutl_sex)                                       \
          {                                                    \
             recutl_fatal (_("cannot specify -e and also -n\n"));\
          }                                                    \
        if (recutl_num_indexes() != 0)                         \
          {                                                    \
            recutl_fatal (_("cannot specify -e and also -n\n"));\
          }                                                    \
                                                               \
        recutl_quick_str = xstrdup (optarg);                   \
        break;                                                 \
      }                                                        \
      case INSENSITIVE_ARG:                                    \
      case 'i':                                                \
      {                                                        \
          recutl_insensitive = true;                           \
          break;                                               \
      }

#if defined REC_CRYPT_SUPPORT
#  define ENCRYPTION_SHORT_ARGS "s:"
#else
#  define ENCRYPTION_SHORT_ARGS ""
#endif

/*
 * Function prototypes.
 */

void recutl_init (char *util_name);
void recutl_print_version (void);
void recutl_print_help_common (void);
void recutl_print_help_footer (void);
void recutl_print_help_record_selection (void);


void recutl_error (const char *fmt, ...);
void recutl_warning (const char *fmt, ...);
void recutl_fatal (const char *fmt, ...);
void recutl_out_of_memory (void);

bool recutl_parse_db_from_file (FILE *in, char *file_name, rec_db_t db);
rec_db_t recutl_build_db (int argc, char **argv);

rec_db_t recutl_read_db_from_file (char *file_name);
void recutl_write_db_to_file (rec_db_t db, char *file_name);

bool recutl_file_is_writable (char *file_name);

char *recutl_read_file (char *file_name);

void recutl_check_integrity (rec_db_t db,
                             bool verbose_p,
                             bool external_p);

bool recutl_yesno (char *prompt);

bool recutl_interactive (void);

char *recutl_getpass (bool asktwice);

/* Parse a list of indexes from the given string and set the internal
   recutl_indexes accordingly.  Return true if a list was found in the
   string.  Return false otherwise.  */

bool recutl_index_list_parse (const char *str);

/* Return the number of indexes in the internal recutl_indexes data
   structure.  */

size_t recutl_num_indexes (void);

/* Reset the list of indexes to be empty.  */

void recutl_reset_indexes (void);

/* Return the index structure.  */

size_t *recutl_index (void);

#endif /* recutl.h */

/* End of recutl.h */
