#!/bin/sh
#
# recdel.sh - System tests for recdel.
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
test_init "recdel"

#
# Create input files.
#

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

test_declare_input_file integrity-fail \
'%rec: IntegrityFail
%type: Id int

Id: foo

Id: bar
'

test_declare_input_file external-types \
'%rec: External
%type: Id int
'

test_declare_input_file external \
'%rec: External external-types.in

Id: foo

Id: bar
'

#
# Declare tests.
#

test_tool recdel-first ok \
          recdel \
          '-n 0' \
          multiple-records \
'field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33
'

test_tool recdel-second ok \
          recdel \
          '-n 1' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value31
field2: value32
field3: value33
'

test_tool recdel-last ok \
          recdel \
          '-n 2' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value22
field3: value23
'

test_tool recdel-index-list ok \
          recdel \
          '-n 0,2' \
          multiple-records \
'field1: value21
field2: value22
field3: value23
'

test_tool recdel-case-insensitive ok \
          recdel \
          '-i -q Value22' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value31
field2: value32
field3: value33
'

test_tool recdel-case-sensitive ok \
          recdel \
          '-q Value22' \
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
'

test_tool recdel-comment ok \
          recdel \
          '-n 1 -c' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

#field1: value21
#field2: value22
#field3: value23

field1: value31
field2: value32
field3: value33
'

test_tool recdel-sex ok \
          recdel \
          '-e "field2 = '\''value22'\''"' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value31
field2: value32
field3: value33
'

test_tool recdel-try-type xfail \
          recdel \
          '-t Type2' \
          multiple-named

test_tool recdel-type ok \
          recdel \
          '--force -t Type2' \
          multiple-named \
'%rec: Type1

field1: value11
field2: value12
field3: value13

%rec: Type2

%rec: Type3

field1: value31
field2: value32
field3: value33
'

test_tool recdel-request-all xfail \
          recdel \
          '' \
          multiple-records

test_tool recdel-force-all ok \
          recdel \
          '--force' \
          multiple-records \
''

test_tool recdel-integrity-fail xfail \
          recdel \
          '-t IntegrityFail -n 0' \
          integrity-fail

test_tool recdel-force-restrictions ok \
          recdel \
          '--force -t IntegrityFail -n 0' \
          integrity-fail \
'%rec: IntegrityFail
%type: Id int

Id: bar
'

test_tool recdel-external-fail xfail \
          recdel \
          '-t External -n 0' \
          external

test_tool recdel-no-external ok \
          recdel \
          '--no-external -t External -n 0' \
          external \
'%rec: External external-types.in

Id: bar
'

test_tool recdel-quick-simple ok \
          recdel \
          '-q value22' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value31
field2: value32
field3: value33
'

test_tool recdel-random-all xfail \
          recdel \
          '-m 0' \
          multiple-records

test_tool recdel-random-all-force ok \
          recdel \
          '-m 0 --force' \
          multiple-records \
''

#
# Cleanup
#

test_cleanup
exit $?

# End of recdel.sh
