/* -*- mode: C -*-
 *
 *       File:         rec-utils.h
 *       Date:         Fri Apr  9 19:42:52 2010
 *
 *       GNU recutils - Miscellanea utilities
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

#ifndef REC_UTILS_H
#define REC_UTILS_H

#include <config.h>

#include <rec.h>
#include <stdint.h>
#include <stdbool.h>

enum rec_size_condition_e
  {
    SIZE_COND_E,
    SIZE_COND_L,
    SIZE_COND_LE,
    SIZE_COND_G,
    SIZE_COND_GE
  };

/* Non public constants used by several modules. */

#define REC_INT_RE "-?((0x[0-9a-fA-F]+)|[0-9]+)"
#define REC_INT_SIZE_RE                         \
  "^[ \t\n]*(>|<|>=|<=)?[ \t\n]*"               \
  REC_INT_RE                                    \
  "[ \t\n]*$"

/* Parse an integer/real in the NULL-terminated string STR and store
   it at NUMBER.  Return true if the conversion was successful.  false
   otherwise. */
bool rec_atoi (const char *str, int *number);
bool rec_atod (const char *str, double *number);

/* Extract type and url from a %rec: field value.  */
char *rec_extract_url (const char *str);
char *rec_extract_file (const char *str);
char *rec_extract_type (const char *str);

/* Extract size and condition from a %size: field value.  */
size_t rec_extract_size (const char *str);
enum rec_size_condition_e rec_extract_size_condition (const char *str);

/* Matching a string against a regexp.  */
bool rec_match (const char *str, const char *regexp);
bool rec_match_insensitive (const char *str, const char *regexp);

/* Generic parsing routines.  */
bool rec_blank_p (char c);
bool rec_digit_p (char c);
bool rec_letter_p (char c);
bool rec_parse_int (const char **str, int *num);
void rec_skip_blanks (const char **str);
bool rec_parse_regexp (const char **str, const char *re, char **result);

/* String utilities.  */
char *rec_concat_strings (const char *str1, const char *str2, const char *str3);

/* Miscellanea.  */
int rec_timespec_subtract (struct timespec *result,
                           struct timespec *x,
                           struct timespec *y);

uint32_t rec_endian_swap (uint32_t number);

#endif /* rec-utils.h */

/* End of rec-utils.h.  */
