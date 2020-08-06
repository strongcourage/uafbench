#!/bin/sh
#
# recins.sh - System tests for recins.
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
test_init "recins"

#
# Create input files.
#

test_declare_input_file empty-file \
''

test_declare_input_file one-record \
'field1: value11
field2: value12
field3: value13
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

test_declare_input_file multiple-records-case \
'field1: Value11
field2: Value12
field3: Value13

field1: Value21
field2: Value22
field3: Value23

field1: Value31
field2: Value32
field3: Value33
'

test_declare_input_file multiple-named \
'%rec: Type1

field1: value11
field2: value12
field3: value13

%rec: Type2

field1: value21
field2: value22
field3: value23

%rec: Type3

field1: value31
field2: value32
field3: value33
'

test_declare_input_file integrity \
'%rec: Integrity
%type: Id int

Id: 0
'

test_declare_input_file only-descriptor \
'%rec: foo
'

test_declare_input_file comments-and-descriptor \
'# comment 1

# comment 2

%rec: foo
'

test_declare_input_file external-descriptor-types \
'%rec: Patata
%type: foo int
' 

test_declare_input_file external-descriptor \
'%rec: Patata recins-external-descriptor-types.in

foo: 10
'

test_declare_input_file auto-fields \
'%rec: foo
%type: myint int
%type: myrange range 0 10
%auto: myint myrange

myint: 10
myrange: 0
'

test_declare_input_file auto-notype \
'%rec: foo
%auto: key

key: 12
'

test_declare_input_file auto-fields-several \
'%rec: foo
%type: myint int
%type: myrange range 0 10
%auto: myint
%auto: myrange

myint: 10
myrange: 0
'

test_declare_input_file auto-range-overflow \
'%rec: foo
%type: myrange range 0 10
%auto: myrange

myrange: 10
'

test_declare_input_file marks \
'%rec: Marks
%type: Class enum A B C

Name: Alfred
Class: A

Name: Bertram
Class: B
'

#
# Declare tests.
#

test_tool recins-empty ok \
          recins \
          '-f field1 -v "value1"' \
          empty-file \
'field1: value1
'

test_tool recins-empty-with-type ok \
          recins \
          '-t Type1 -f field1 -v "value1"' \
          empty-file \
'%rec: Type1

field1: value1
'

test_tool recins-several-fields ok \
          recins \
          '-f field1 -v "value1" -f field2 -v "value2"' \
          empty-file \
'field1: value1
field2: value2
'

test_tool recins-append ok \
          recins \
          '-f afield1 -v "appended1"' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33

afield1: appended1
'

test_tool recins-append-in-type ok \
          recins \
          '-t Type2 -f afield1 -v "appended1"' \
          multiple-named \
'%rec: Type1

field1: value11
field2: value12
field3: value13

%rec: Type2

field1: value21
field2: value22
field3: value23

afield1: appended1

%rec: Type3

field1: value31
field2: value32
field3: value33
'

test_tool recins-append-new-type ok \
          recins \
          '-t NewType -f afield1 -v "appended1"' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33

%rec: NewType

afield1: appended1
'

test_tool recins-replace ok \
          recins \
          '-n 1 -f afield1 -v "replaced"' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

afield1: replaced

field1: value31
field2: value32
field3: value33
'

test_tool recins-replace-range ok \
          recins \
          '-n 1-2 -f afield1 -v "replaced"' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

afield1: replaced

afield1: replaced
'

test_tool recins-replace-several-indexes ok \
          recins \
          '-n 0,2 -f afield1 -v "replaced"' \
          multiple-records \
'afield1: replaced

field1: value21
field2: value22
field3: value23

afield1: replaced
'

test_tool recins-replace-sex ok \
          recins \
          '-e "field2 = '\''value22'\''" -f afield1 -v "replaced"' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

afield1: replaced

field1: value31
field2: value32
field3: value33
'

test_tool recins-violate-restrictions xfail \
          recins \
          '-t Integrity -f Id -v "not a number"' \
          integrity

test_tool recins-force-restrictions ok \
          recins \
          '--force -t Integrity -f Id -v "not a number"' \
          integrity \
'%rec: Integrity
%type: Id int

Id: 0

Id: not a number
'

test_tool recins-only-descriptor ok \
          recins \
          '' \
          only-descriptor \
'%rec: foo
'

test_tool recins-comments-and-descriptor ok \
          recins \
          '' \
          comments-and-descriptor \
'# comment 1

# comment 2

%rec: foo
'

test_tool recins-external-descriptor ok \
          recins \
          '-t Patata -f foo -v 20' \
          external-descriptor \
'%rec: Patata recins-external-descriptor-types.in

foo: 10

foo: 20
'

test_tool recins-auto-fields ok \
          recins \
          '-t foo -f foo -v bar' \
          auto-fields \
'%rec: foo
%type: myint int
%type: myrange range 0 10
%auto: myint myrange

myint: 10
myrange: 0

myint: 11
myrange: 1
foo: bar
'

test_tool recins-auto-notype ok \
          recins \
          '-t foo -f foo -v bar' \
          auto-notype \
'%rec: foo
%auto: key

key: 12

key: 13
foo: bar
'

test_tool recins-auto-fields-several ok \
          recins \
          '-t foo -f foo -v bar' \
          auto-fields-several \
'%rec: foo
%type: myint int
%type: myrange range 0 10
%auto: myint
%auto: myrange

myint: 10
myrange: 0

myint: 11
myrange: 1
foo: bar
'

test_tool recins-no-auto ok \
          recins \
          '--no-auto -t foo -f foo -v bar' \
          auto-fields \
'%rec: foo
%type: myint int
%type: myrange range 0 10
%auto: myint myrange

myint: 10
myrange: 0

foo: bar
'

test_tool recins-auto-range-overflow xfail \
          recins \
          '-t foo -f foo -v bar' \
          auto-range-overflow

test_tool recins-auto-range-overflow-force ok \
          recins \
          '--force -t foo -f foo -v bar' \
          auto-range-overflow \
'%rec: foo
%type: myrange range 0 10
%auto: myrange

myrange: 10

myrange: 11
foo: bar
'

test_tool recins-quick-simple ok \
          recins \
          '-q value22 -f foo -v bar' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

foo: bar

field1: value31
field2: value32
field3: value33
'

test_tool recins-random-all ok \
          recins \
          '-m 0 -f foo -v bar' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33

foo: bar
'

test_tool recins-random-one ok \
          recins \
          '-m 1 -f foo -v bar' \
          one-record \
'foo: bar
'

test_tool recins-case-insensitive ok \
          recins \
          '-i -q value22 -f foo -v bar' \
          multiple-records-case \
'field1: Value11
field2: Value12
field3: Value13

foo: bar

field1: Value31
field2: Value32
field3: Value33
'

test_tool recins-case-sensitive ok \
          recins \
          '-q value22 -f foo -v bar' \
          multiple-records-case \
'field1: Value11
field2: Value12
field3: Value13

field1: Value21
field2: Value22
field3: Value23

field1: Value31
field2: Value32
field3: Value33
'

test_tool recins-insert-default-type ok \
          recins \
          '-f Name -v Xavier -f Class -v X' \
          marks \
'Name: Xavier
Class: X

%rec: Marks
%type: Class enum A B C

Name: Alfred
Class: A

Name: Bertram
Class: B
'
 
#
# Cleanup.
#

test_cleanup
exit $?

# End of recins.sh
