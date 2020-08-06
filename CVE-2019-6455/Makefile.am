# Top-level Makefile.am

# Copyright (C) 2009-2015 Jose E. Marchesi

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

ACLOCAL_AMFLAGS = -I m4
SUBDIRS = lib libcsv src utils bash doc po man torture etc

if BUILD_A68
   SUBDIRS += algol68
endif

perf:
	$(MAKE) -C torture/utils perf

.PHONY: perf

# End of Makefile.am
