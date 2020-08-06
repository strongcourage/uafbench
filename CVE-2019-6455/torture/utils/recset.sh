#!/bin/sh
#
# recset.sh - System tests for recset.
#
# Copyright (C) 2010-2015 Jose E. Marchesi.
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
test_init "recset"

#
# Create input files.
#

test_declare_input_file one-record \
'field1: value1
field2: value2
field3: value3
'

test_declare_input_file repeated-fields \
'field1: value1
field2: value21
field2: value22
field3: value3
'

test_declare_input_file integrity \
'%rec: Integrity
%key: Id

Id: 10
other: field
'

test_declare_input_file renames \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
field2: field121
field2: field122
field2: field123
field3: field13
'

test_declare_input_file renames-multiple \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
field2: field121
field2: field122
field2: field123
field3: field13

field1: field21
field2: field221
field2: field222
field2: field223
field3: field23
'

test_declare_input_file multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33
'

#
# Declare tests.
#

test_tool recset-append-field ok \
          recset \
          '-n 0 -f foo -a bar' \
          one-record \
'field1: value1
field2: value2
field3: value3
foo: bar
'

test_tool recset-set-field ok \
          recset \
          '-n 0 -f field2 -s bar' \
          one-record \
'field1: value1
field2: bar
field3: value3
'

test_tool recset-delete-field ok \
          recset \
          '-n 0 -f field2 -d' \
          one-record \
'field1: value1
field3: value3
'

test_tool recset-comment-out-field ok \
          recset \
          '-n 0 -f field2 -c' \
          one-record \
'field1: value1
#field2: value2
field3: value3
'

test_tool recset-delete-non-existant ok \
          recset \
          '-n 0 -f nonexistant -d' \
          one-record \
'field1: value1
field2: value2
field3: value3
'

test_tool recset-comment-out-fex-first ok \
          recset \
          '-n 0 -f field2[0] -c' \
          repeated-fields \
'field1: value1
#field2: value21
field2: value22
field3: value3
'

test_tool recset-comment-out-fex-last ok \
          recset \
          '-n 0 -f field2[1] -c' \
          repeated-fields \
'field1: value1
field2: value21
#field2: value22
field3: value3
'

test_tool recset-violate-integrity xfail \
          recset \
          '-n 0 -f Id -d' \
          integrity

test_tool recset-force-integrity ok \
          recset \
          '--force -n 0 -f Id -d' \
          integrity \
'%rec: Integrity
%key: Id

other: field
'

test_tool recset-rename ok \
          recset \
          '--force -t Type1 -n 0 -f field2 -r foo' \
          renames \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
foo: field121
foo: field122
foo: field123
field3: field13
'

test_tool recset-rename-first ok \
          recset \
          '--force -t Type1 -n 0 -f field2[0] -r foo' \
          renames \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
foo: field121
field2: field122
field2: field123
field3: field13
'

test_tool recset-rename-middle ok \
          recset \
          '--force -t Type1 -n 0 -f field2[1] -r foo' \
          renames \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
field2: field121
foo: field122
field2: field123
field3: field13
'

test_tool recset-rename-last ok \
          recset \
          '--force -t Type1 -n 0 -f field2[2] -r foo' \
          renames \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
field2: field121
field2: field122
foo: field123
field3: field13
'

test_tool recset-rename-range-first ok \
          recset \
          '--force -t Type1 -n 0 -f field2[0-1] -r foo' \
          renames \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
foo: field121
foo: field122
field2: field123
field3: field13
'

test_tool recset-rename-range-last ok \
          recset \
          '--force -t Type1 -n 0 -f field2[1-2] -r foo' \
          renames \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
field2: field121
foo: field122
foo: field123
field3: field13
'

test_tool recset-rename-range-all ok \
          recset \
          '--force -t Type1 -n 0 -f field2[0-2] -r foo' \
          renames \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
foo: field121
foo: field122
foo: field123
field3: field13
'

test_tool recset-rename-all ok \
          recset \
          '--force -t Type1 -e "1" -f field2[0-2] -r foo' \
          renames-multiple \
'%rec: Type1
%key: field1
%type: field2 line
%mandatory: field2
%sort: field2

field1: field11
foo: field121
foo: field122
foo: field123
field3: field13

field1: field21
foo: field221
foo: field222
foo: field223
field3: field23
'

test_tool recset-rename-all-descriptor ok \
          recset \
          '--force -t Type1 -f field2[0-2] -r foo' \
          renames-multiple \
'%rec: Type1
%key: field1
%type: foo line
%mandatory: foo
%sort: foo

field1: field11
foo: field121
foo: field122
foo: field123
field3: field13

field1: field21
foo: field221
foo: field222
foo: field223
field3: field23
'

test_tool recset-rename-all-key ok \
          recset \
          '--force -t Type1 -f field1 -r newkey' \
          renames-multiple \
'%rec: Type1
%key: newkey
%type: field2 line
%mandatory: field2
%sort: field2

newkey: field11
field2: field121
field2: field122
field2: field123
field3: field13

newkey: field21
field2: field221
field2: field222
field2: field223
field3: field23
'

test_tool recset-rename-invalid-fex xfail \
          recset \
          '--force -t Type1 -f field1,field2 -r foo' \
          renames-multiple

test_tool recset-set-add-1 ok \
          recset \
          '-f field2 -S foo' \
          one-record \
'field1: value1
field2: foo
field3: value3
'

test_tool recset-set-add-2 ok \
          recset \
          '-f field4 -S foo' \
          one-record \
'field1: value1
field2: value2
field3: value3
field4: foo
'

test_tool recset-set-add-3 ok \
          recset \
          '-f field2,field4 -S foo' \
          one-record \
'field1: value1
field2: foo
field3: value3
field4: foo
'

test_tool recset-quick-simple ok \
          recset \
          '-q value22 -f field1 -s foo' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: foo
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33
'

test_tool recset-set-field-in-range ok \
          recset \
          '-n 0-1 -f field2 -s XXX' \
          multiple-records \
'field1: value11
field2: XXX
field3: value13

field1: value21
field2: XXX
field3: value23

field1: value31
field2: value32
field3: value33
'

test_tool recset-set-field-in-several-indexes ok \
          recset \
          '-n 0,2 -f field2 -s XXX' \
          multiple-records \
'field1: value11
field2: XXX
field3: value13

field1: value21
field2: value22
field3: value23

field1: value31
field2: XXX
field3: value33
'

test_tool recset-random-all ok \
          recset \
          '-m 0 -f field2 -s XXX' \
          multiple-records \
'field1: value11
field2: XXX
field3: value13

field1: value21
field2: XXX
field3: value23

field1: value31
field2: XXX
field3: value33
'

test_tool recset-random-one ok \
          recset \
          '-m 1 -f field2 -s XXX' \
          one-record \
'field1: value1
field2: XXX
field3: value3
'
 
#
# Cleanup.
#

test_cleanup
exit $?

# End of recset.sh
