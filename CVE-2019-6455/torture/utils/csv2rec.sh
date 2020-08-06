#!/bin/sh
#
# csv2rec.sh - System tests for csv2rec.
#
# Copyright (C) 2011-2015 Jose E. Marchesi.
#
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

#
# Initialization
#

: ${srcdir=.}
. $srcdir/testutils.sh
test_init "csv2rec"

#
# Create input files.
#

test_declare_input_file simple-table \
'a,b,c
a1,b1,c1
a2,b2,c2
a3,b3,c3
'

test_declare_input_file simple-table-quotes \
'"a","b","c"
"a1","b1","c1"
a2,b2,c2
"a3","b3",c3
'

test_declare_input_file empty-columns \
'"a","b","c"
,b1,c1
a2,,c2
a3,b3,
'

test_declare_input_file multi-line \
'a,b
"foo
bar",baz
x,y
'

#
# Declare tests.
#

test_tool csv2rec-simple-table ok \
          csv2rec \
          '' \
          simple-table \
'a: a1
b: b1
c: c1

a: a2
b: b2
c: c2

a: a3
b: b3
c: c3
'

test_tool csv2rec-simple-table-quotes ok \
          csv2rec \
          '' \
          simple-table-quotes \
'a: a1
b: b1
c: c1

a: a2
b: b2
c: c2

a: a3
b: b3
c: c3
'

test_tool csv2rec-empty-columns ok \
          csv2rec \
          '' \
          empty-columns \
'a:
b: b1
c: c1

a: a2
b:
c: c2

a: a3
b: b3
c:
'

test_tool csv2rec-empty-columns-omit ok \
          csv2rec \
          '-e' \
          empty-columns \
'b: b1
c: c1

a: a2
c: c2

a: a3
b: b3
'

test_tool csv2rec-multi-line ok \
          csv2rec \
          '' \
          multi-line \
'a: foo
+ bar
b: baz

a: x
b: y
'

#
# Cleanup
#

test_cleanup
exit $?

# End of csv2rec.sh
