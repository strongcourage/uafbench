/* -*- mode: C -*-
 *
 *       File:         rec.c
 *       Date:         Sun Oct 24 19:47:16 2010
 *
 *       GNU recutils - Initialization and finalization routines.
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

#if defined REMOTE_DESCRIPTORS
#   include <curl/curl.h>
#endif

#include <gettext.h>

#include <rec.h>

void
rec_init (void)
{
  bindtextdomain (PACKAGE, LOCALEDIR);
#if defined REMOTE_DESCRIPTORS
  curl_global_init (CURL_GLOBAL_ALL);
#endif
}

void
rec_fini (void)
{
#if defined REMOTE_DESCRIPTORS
  curl_global_cleanup ();
#endif
}

/* End of rec.c */
