#!/bin/sh
#
# rec2csv.sh - System tests for rec2csv.
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
test_init "rec2csv"

#
# Create input files.
#

test_declare_input_file default-records \
'a: a1
b: b1
c: c1

a: a2
b: b2
c: c2
'

test_declare_input_file missing-fields \
'a: a1
b: b1
c: c1

a: a2
c: c2

b: b3
c: c3

a: a4
b: b4
'

test_declare_input_file missing-fields-first \
'a: a1
c: c1

a: a2
b: b2
c: c2
'

test_declare_input_file multi-line \
'a: foo
+ bar
+ baz
b: jo
+ ju
+ je
'

test_declare_input_file escape-quotes \
'a: foo"bar
b: bar"baz
'

test_declare_input_file repeated-fields \
'a: a11
a: a12

a: a21
a: a22
'

test_declare_input_file repeated-missing \
'a: a11
a: a12

b: b2
a: a21
'

test_declare_input_file several-types \
'a: a_none
b: b_none

%rec: foo

a: a_foo
b: b_foo

%rec: bar

a: a_bar
b: b_bar
'

test_declare_input_file sort \
'%rec: Sorted
%sort: foo
%type: bar int

foo: b
bar: 100

foo: a
bar: 50

foo: c
bar: 0
'

#
# Declare tests.
#

test_tool rec2csv-default-record ok \
          rec2csv \
          '' \
          default-records \
'"a","b","c"
"a1","b1","c1"
"a2","b2","c2"
'

test_tool rec2csv-missing-fields ok \
          rec2csv \
          '' \
          missing-fields \
'"a","b","c"
"a1","b1","c1"
"a2",,"c2"
,"b3","c3"
"a4","b4",
'

test_tool rec2csv-missing-fields-first ok \
          rec2csv \
          '' \
          missing-fields-first \
'"a","c","b"
"a1","c1",
"a2","c2","b2"
'

test_tool rec2csv-multi-line ok \
          rec2csv \
          '' \
          multi-line \
'"a","b"
"foo
bar
baz","jo
ju
je"
'

test_tool rec2csv-escape-quotes ok \
          rec2csv \
          '' \
          escape-quotes \
'"a","b"
"foo""bar","bar""baz"
'

test_tool rec2csv-repeated-fields ok \
          rec2csv \
          '' \
          repeated-fields \
'"a","a_2"
"a11","a12"
"a21","a22"
'

test_tool rec2csv-repeated-missing ok \
          rec2csv \
          '' \
          repeated-missing \
'"a","a_2","b"
"a11","a12",
"a21",,"b2"
'

test_tool rec2csv-default-type ok \
          rec2csv \
          '' \
          several-types \
'"a","b"
"a_none","b_none"
'

test_tool rec2csv-with-type ok \
          rec2csv \
          '-t bar' \
          several-types \
'"a","b"
"a_bar","b_bar"
'

test_tool rec2csv-nonexistant-type ok \
          rec2csv \
          '-t jorl' \
          several-types \
''

test_tool rec2csv-sort ok \
          rec2csv \
          '' \
          sort \
'"foo","bar"
"a","50"
"b","100"
"c","0"
'

test_tool rec2csv-sort-field ok \
          rec2csv \
          '-S bar' \
          sort \
'"foo","bar"
"c","0"
"a","50"
"b","100"
'

#
# Cleanup
#

test_cleanup
exit $?

# End of rec2csv.sh
