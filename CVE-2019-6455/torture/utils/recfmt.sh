#!/bin/sh
#
# recfmt.sh - System tests for recfmt.
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
test_init "recfmt"

#
# Create input files.
#

test_declare_input_file empty-file ''

test_declare_input_file multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value22
field2: value22bis
field3: value23

field1: value31
field2: value32
field3: value33
'

#
# Declare tests
#

# Apply an empty template.
test_tool recfmt-empty-template ok \
          recfmt \
          '""' \
          multiple-records \
''

# Apply a template to an empty file.
test_tool recfmt-empty-file ok \
          recfmt \
          '"a template {{foo}}"' \
          empty-file \
''

# Apply a template affecting all the records.
test_tool recfmt-all-records ok \
          recfmt \
          '"foo {{field3}} bar
"' \
          multiple-records \
'foo value13 bar
foo value23 bar
foo value33 bar
'

# Apply a template with a slot starting at 0.
test_tool recfmt-slot-beginning ok \
          recfmt \
          '"{{field1}} bar
"' \
          multiple-records \
'value11 bar
value21 bar
value31 bar
'

# Apply a template without prolog.
test_tool recfmt-no-prolog ok \
          recfmt \
          '"foo {{field1}}
"' \
          multiple-records \
'foo value11
foo value21
foo value31
'

# Apply a template with subscripts.
test_tool recfmt-subscripts ok \
          recfmt \
          '"foo {{field2[1]}} bar
"' \
          multiple-records \
'foo  bar
foo value22bis bar
foo  bar
'

# Apply a template with non-matching records.
test_tool recfmt-non-matching-records ok \
          recfmt \
          '"{{field2[1]}}"' \
          multiple-records \
'value22bis'

# Apply a template with string concatenation.
test_tool recfmt-concat ok \
          recfmt \
          '"{{field1 & '\'' '\'' & field2 & field3}}
"' \
          multiple-records \
'value11 value12value13
value21 value22value23
value31 value32value33
'

# Try to apply an empty slot.
test_tool recfmt-empty-slot xfail \
          recfmt \
          '"{{field2[1]}}{{}}
"' \
          multiple-records

# Try to apply an invalid slot.
test_tool recfmt-invalid-slot xfail \
          recfmt \
          '"invalid slot: {{in#alid[}}
"' \
          multiple-records

#
# Cleanup
#

test_cleanup
exit $?

# End of recfmt.sh
