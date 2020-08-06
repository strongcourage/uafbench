# Makefile.am for recutils/man

# Copyright (C) 2010 Jose E. Marchesi

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

man_MANS = recdel.1 \
           recfix.1 \
           recinf.1 \
           recins.1 \
           recsel.1 \
           recset.1 \
           recfmt.1 \
           csv2rec.1 \
           rec2csv.1

if COMPILE_MDB2REC
man_MANS += mdb2rec.1
endif

EXTRA_DIST = $(man_MANS)
MAINTAINERCLEANFILES = $(man_MANS)

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac \
                 $(top_srcdir)/utils/recutl.c

# Depend on the source, not the binary; we don't need to regenerate
# the binary when any source file changes, only the main one.
# Use -o so that the `missing' program can infer the output file.

rec2csv.1: $(top_srcdir)/utils/rec2csv.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="rec to csv converter" $(builddir)/../utils/rec2csv -o $@

csv2rec.1: $(top_srcdir)/utils/csv2rec.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="csv to rec converter" $(builddir)/../utils/csv2rec -o $@

mdb2rec.1: $(top_srcdir)/utils/mdb2rec.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="mdb to rec converter" $(builddir)/../utils/mdb2rec -o $@

recdel.1: $(top_srcdir)/utils/recdel.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="remove records from a recfile" $(builddir)/../utils/recdel -o $@

recfix.1: $(top_srcdir)/utils/recfix.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="check a recfile for errors" $(builddir)/../utils/recfix -o $@

recinf.1: $(top_srcdir)/utils/recinf.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="print information about a recfile" $(builddir)/../utils/recinf -o $@

recins.1: $(top_srcdir)/utils/recins.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="insert records in a recfile" $(builddir)/../utils/recins -o $@

recsel.1: $(top_srcdir)/utils/recsel.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="print records from a recfile" $(builddir)/../utils/recsel -o $@

recset.1: $(top_srcdir)/utils/recset.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="manipulate fields in a recfile" $(builddir)/../utils/recset -o $@

recfmt.1: $(top_srcdir)/utils/recfmt.c $(common_mandeps)
	$(HELP2MAN) -p recutils --name="apply a template to records" $(builddir)/../utils/recfmt -o $@

# End of Makefile.am
