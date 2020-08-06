/* -*- mode: C -*- Time-stamp: "2019-01-03 09:46:25 jemarch"
 *
 *       File:         rec-crypt-dummy.c
 *       Date:         Tue Mar 27 21:29:06 2012
 *
 *       GNU recutils - Dummy replacements for encryption routines
 *
 */

/* Copyright (C) 2012-2019 Michał Masłowski */

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

#include <rec.h>

bool
rec_encrypt (char   *in,
             size_t  in_size,
             const char   *password,
             char  **out,
             size_t *out_size)
{
  return false;
}

bool
rec_decrypt (char   *in,
             size_t  in_size,
             const char   *password,
             char  **out,
             size_t *out_size)
{
  return false;
}

bool
rec_encrypt_record (rec_rset_t rset,
                    rec_record_t record,
                    const char *password)
{
  return false;
}

bool
rec_encrypt_field (rec_field_t field,
                   const char *password)
{
  return false;
}

bool
rec_decrypt_field (rec_field_t field,
                   const char *password)
{
  return false;
}

bool
rec_decrypt_record (rec_rset_t rset,
                    rec_record_t record,
                    const char *password)
{
  return false;
}

/* End of rec-crypt-dummy.c */
