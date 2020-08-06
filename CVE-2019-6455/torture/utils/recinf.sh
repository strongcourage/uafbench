#!/bin/sh
#
# recinf.sh - System tests for recinf.
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
test_init "recinf"

#
# Create input files.
#

test_declare_input_file empty-file ''

test_declare_input_file invalid \
'foo: bar
bar: baz

+ jo
ju: bar
'

test_declare_input_file one-record \
'field1: value1
field2: value2
field3: value3
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

#
# Declare tests.
#

test_tool recinf-empty ok \
          recinf \
          '' \
          empty-file \
''

test_tool recinf-one-record ok \
          recinf \
          '' \
          one-record \
'1
'

test_tool recinf-multiple-records ok \
          recinf \
          '' \
          multiple-records \
'3
'

test_tool recinf-multiple-named ok \
          recinf \
          '' \
          multiple-named \
'1 Type1
1 Type2
1 Type3
'

test_tool recinf-multiple-named-descriptors ok \
          recinf \
          '-d' \
          multiple-named \
'%rec: Type1

%rec: Type2

%rec: Type3
'

test_tool recinf-multiple-types ok \
          recinf \
          '-t Type2' \
          multiple-named \
'1 Type2
'

test_tool recinf-names-only ok \
          recinf \
          '-n' \
          multiple-named \
'Type1
Type2
Type3
'

test_tool recinf-invalid xfail \
          recinf \
          '' \
          invalid

#
# Cleanup
#

test_cleanup
exit $?

# End of recinf.sh
