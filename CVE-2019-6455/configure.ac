dnl configure.ac for GNU rec
dnl
dnl Please process this file with autoconf to get a 'configure'
dnl script.

dnl Copyright (C) 2009-2019 Jose E. Marchesi

dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_INIT([GNU recutils], [1.8], [bug-recutils@gnu.org])

dnl Must come before AM_INIT_AUTOMAKE
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_MACRO_DIR([m4])

dnl Autobuild
AB_INIT

AC_PROG_CC
gl_EARLY

LT_INIT
AC_PROG_CC_C99
AM_PROG_CC_C_O

dnl Both lex and yacc are required to generate the lexer/parser source
dnl files.

: ${LEX='flex'}
: ${LEXLIB='-lfl'}
: ${LEX_OUTPUT_ROOT='lex.yy'}
AC_ARG_VAR([LEX], [The flex implementation to use.])
AC_ARG_VAR([LEXLIB], [Options for linking with the flex runtime library.])
AC_ARG_VAR([LEX_OUTPUT_ROOT], [Base of the file name that the lexer generates.])

gl_BISON

dnl System
AC_CANONICAL_HOST
canonical=$host

gl_INIT

# GNU help2man creates man pages from --help output; in many cases, this
# is sufficient, and obviates the need to maintain man pages separately.
# However, this means invoking executables, which we generally cannot do
# when cross-compiling, so we test to avoid that (the variable
# "cross_compiling" is set by AC_PROG_CC).
if test $cross_compiling = no; then
  AM_MISSING_PROG(HELP2MAN, help2man)
else
  HELP2MAN=:
fi

dnl Seach for headers
AC_CHECK_HEADERS([malloc.h string.h])

dnl Search for data types
AC_CHECK_TYPE(size_t, unsigned)
AC_CHECK_TYPE(off_t, long)

dnl Search for functions
AC_FUNC_FSEEKO

dnl Search for required libraries

have_check=no
PKG_CHECK_MODULES([CHECK], [check],
                  [have_check=yes], [have_check=no])
AM_CONDITIONAL([CHECK], [test "x$have_check" != "xno"])
AC_SUBST([CHECK_CFLAGS])
AC_SUBST([CHECK_LIBS])

have_curl=no
AC_CHECK_LIB([curl],[curl_global_init],[have_curl=yes],)
if test "x$have_curl" = "xyes"; then
   CURLLIBS=-lcurl
fi
AC_SUBST([CURLLIBS])

have_uuid=no
AC_CHECK_LIB([uuid],[uuid_generate],[have_uuid=yes],)
if test "x$have_uuid" = "xyes"; then
   UUIDLIBS=-luuid
fi
AC_SUBST([UUIDLIBS])
AC_SUBST([have_uuid])

AC_ARG_ENABLE([encryption],
              AS_HELP_STRING([--enable-encryption],
                             [Compile recutils with encryption support (default is YES)]),
              [crypt_enabled=$enableval], [crypt_enabled=yes])

crypt_support=no
if test "x$crypt_enabled" = "xyes"; then

   AC_LIB_HAVE_LINKFLAGS([gcrypt],[gpg-error],[#include <gpg-error.h>])
   crypt_support=$HAVE_LIBGCRYPT

   if test "x$crypt_support" = "xyes"; then
      AC_DEFINE([REC_CRYPT_SUPPORT],[1],[Compile encryption support])
   fi
fi

AM_CONDITIONAL([CRYPT], [test "x$crypt_support" = "xyes"])
AC_SUBST([crypt_support])

have_glib=no
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.0.0],
                  [have_glib=yes], [have_glib=no])

have_mdb=no
AC_CHECK_LIB([mdb],[mdb_init],[have_mdb=yes],)
if test "x$have_mdb" = "xyes"; then
   MDBLIBS=-lmdb

   OLD_CFLAGS=$CFLAGS
   CFLAGS="$CFLAGS $pkg_cv_GLIB_CFLAGS"
   mdb_uses_sdatetime=no
   AC_CHECK_DECL([MDB_SDATETIME],[mdb_uses_sdatetime=yes],,[#include <glib.h>
#include <mdbtools.h>])
   CFLAGS=$OLD_CFLAGS

   if test "x$mdb_uses_sdatetime" = "xyes"; then
      MDB_DATETIME="MDB_SDATETIME"
   else
      MDB_DATETIME="MDB_DATETIME"
   fi

   AC_SUBST([MDB_DATETIME])
fi
AC_SUBST([MDBLIBS])

AM_CONDITIONAL([COMPILE_MDB2REC],
               [test "x$have_glib" = "xyes" && test "x$have_mdb" = "xyes"])
AM_CONDITIONAL([REMOTE_DESCRIPTORS], [test "x$have_curl" = "xyes"])
AM_CONDITIONAL([UUID_TYPE], [test "x$have_uuid" = "xyes"])

dnl Bash builtins

dnl It would be much better to use AC_CHECK_HEADER([bash/config.h]) instead
dnl of a fixed value like /usr/include/bash, but then it would be difficult
dnl to set a proper search path for the preprocessor, since the bash
dnl headers which are needed to compile loadable builtins are not
dnl very well designed.

BASH_HEADERS=/usr/include/bash
AC_ARG_WITH([bash-headers],
            AS_HELP_STRING([--with-bash-headers],
                           [location of the bash header files (default is /usr/include/bash)]),
            [BASH_HEADERS=$withval],)

if test -f ${BASH_HEADERS}/config.h; then
   AC_SUBST([BASH_HEADERS])
   bash_headers_available=yes
else
   bash_headers_available=no
fi

AC_ARG_ENABLE([bash-builtins],
              AS_HELP_STRING([--enable-bash-builtins],
                             [Build the recutils bash builtins (default is YES)]),
              [bash_builtins_enabled=$enableval], [bash_builtins_enabled=yes])

AM_CONDITIONAL([BASH_BUILTINS],
               [test "x$bash_headers_available" = "xyes" && test "x$bash_builtins_enabled" = "xyes"])

dnl Check for an Algol 68 compiler

AC_ARG_ENABLE([algol68],
              AS_HELP_STRING([--enable-algol68],
                             [Build the bindings for Algol 68 (default is NO)]),
              [algol68_enabled=$enableval], [algol68_enabled=no])

if test "x$algol68_enabled" = "xyes"; then
   AC_CHECK_PROGS([CA68], [ca68])
fi

AC_SUBST([CA68])
AM_CONDITIONAL([BUILD_A68],[test -n "$CA68" && test "x$algol68_enabled" = "xyes"])

dnl Platform-based compilation options
compile_w32_system=no
case "${host}" in
     *-mingw32*)
          compile_w32_system=yes
          ;;
     *)
          ;;
esac

dnl i18n with gettext
AM_GNU_GETTEXT_VERSION([0.19.8])
AM_GNU_GETTEXT([external])

dnl gcov compilation
AC_ARG_ENABLE([coverage],
              AS_HELP_STRING([--enable-coverage],
                             [Compile the library with code coverage support (default is NO)]),
              [use_gcov=$enableval], [use_gcov=no])
AM_CONDITIONAL([USE_COVERAGE], [test "x$use_gcov" = "xyes"])

dnl Generate output files
AC_CONFIG_FILES(Makefile
                lib/Makefile
                libcsv/Makefile
                src/Makefile
                utils/Makefile
                bash/Makefile
                doc/Makefile
                po/Makefile.in
                torture/Makefile
                torture/utils/Makefile
                torture/utils/config.sh
                man/Makefile
                etc/Makefile
                algol68/Makefile)
AC_OUTPUT

dnl Report warnings

if test "x$have_check" = "xno"; then
   echo "warning: libcheck was not found in the system."
   echo "warning: unit tests wont be compiled and executed upon make check."
fi

if test "x$have_mdb" = "xno"; then
   echo "warning: libmdb was not found in the system."
   echo "warning: the mdb2rec utility won't get built."
fi

if test "x$have_glib" = "xno"; then
   echo "warning: glib was not found in the system."
   echo "warning: the mdb2rec utility won't get built."
fi

if test "x$crypt_support" = "xno"; then
   echo "warning: building recutils without encryption support."
fi

if test "x$have_uuid" = "xno"; then
   echo "warning: building recutils without support for uuid types."
fi

if test "x$bash_headers_available" = "xno" || test "x$bash_builtins_enabled" = "xno"; then
   echo "warning: not building the recutils bash builtins."
fi

dnl End of configure.ac
