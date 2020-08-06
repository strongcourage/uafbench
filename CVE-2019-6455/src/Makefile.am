# src/ Makefile.am
# GNU recutils

# Copyright (C) 2009-2019 Jose E. Marchesi

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

lib_LTLIBRARIES = librec.la

include_HEADERS = rec.h

librec_la_SOURCES = rec.c \
                    rec-mset.c \
                    rec-utils.h \
                    rec-utils.c \
                    rec-comment.c \
                    rec-field-name.c \
                    rec-field.c \
                    rec-record.c \
                    rec-rset.c \
                    rec-db.c \
                    rec-parser.c \
                    rec-writer.c \
                    rec-int.c \
                    rec-sex-parser.h \
                    rec-sex-parser.c \
                    rec-sex-ast.h \
                    rec-sex-ast.c \
                    rec-sex-tab.y \
                    rec-sex-lex.l \
                    rec-sex.c \
                    rec-fex.c \
                    rec-types.c \
                    rec-buf.c \
                    rec-aggregate.c

if CRYPT
   librec_la_SOURCES += rec-crypt.c
else
   librec_la_SOURCES += rec-crypt-dummy.c
endif

BUILT_SOURCES = rec-sex-tab.h
AM_YFLAGS = -d
AM_LFLAGS = -d
AM_CFLAGS = 
if USE_COVERAGE
  AM_CFLAGS += -fprofile-arcs -ftest-coverage
endif

AM_LDFLAGS = -no-undefined
AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib \
              -DLOCALEDIR=\"$(localedir)\"
librec_la_LDFLAGS = -version-info 1:0:0
librec_la_LIBADD = $(top_builddir)/lib/librecutils.la \
                   $(LIB_CLOCK_GETTIME)

if CRYPT
   librec_la_LIBADD += $(LTLIBGCRYPT)
endif

if REMOTE_DESCRIPTORS
  AM_CPPFLAGS += -DREMOTE_DESCRIPTORS
  librec_la_LIBADD += $(CURLLIBS)
endif

if UUID_TYPE
  AM_CPPFLAGS += -DUUID_TYPE
  librec_la_LIBADD += $(UUIDLIBS)
endif

check-syntax: all

# End of Makefile.am
