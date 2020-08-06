# Makefile.am for recutils/algol68

# Copyright (C) 2011-2019 Jose E. Marchesi

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

lib_LIBRARIES = librec-a68.a
nodist_librec_a68_a_SOURCES = rec.c
BUILT_SOURCES = rec.c
usrlibdir = $(libdir)
usrlib_DATA = rec.m

rec.c: rec.a68
	$(CA68) -C -m librec-a68.a -d $(builddir) -M $(builddir) -I$(top_srcdir)/src -u AAAAGNU $<

clean-local:
	-rm -f $(builddir)/rec.[ocm]

# End of Makefile.am
