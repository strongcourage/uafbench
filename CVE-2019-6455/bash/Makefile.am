# bash/Makefile.am
# GNU recutils

# Copyright (C) 2013-2018 Jose E. Marchesi

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

if BASH_BUILTINS

COMMON_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib \
                  -I$(top_srcdir)/src -DHAVE_CONFIG_H \
                  -I$(BASH_HEADERS) -I$(BASH_HEADERS)/lib -I$(BASH_HEADERS)/builtins \
                  -I$(BASH_HEADERS)/include

lib_LTLIBRARIES = readrec.la testrec.la

AM_CPPFLAGS = $(COMMON_CPPFLAGS)

readrec_la_LDFLAGS = -module
readrec_la_LIBADD = $(top_builddir)/src/librec.la
readrec_la_SOURCES = readrec.c

testrec_la_LDFLAGS = -module
testrec_la_LIBADD = $(top_builddir)/src/librec.la
testrec_la_SOURCES = testrec.c

endif

# End of Makefile.am
