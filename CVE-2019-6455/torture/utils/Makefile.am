# Makefile for torture/utils.
#
# Copyright (C) 2010-2015 Jose E. Marchesi.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TESTS = recinf.sh \
        recsel.sh \
        recdel.sh \
        recins.sh \
        recset.sh \
        recfix.sh \
        recfmt.sh \
        rec2csv.sh \
        csv2rec.sh
EXTRA_DIST = $(TESTS) testutils.sh
TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) builddir=$(builddir) \
                    PATH=$(srcdir)$(PATH_SEPARATOR)$(top_builddir)/utils$(PATH_SEPARATOR)$$PATH

perf:
	 $(TESTS_ENVIRONMENT) srcdir=$(srcdir) $(srcdir)/p-recsel.sh

# End of Makefile.am
