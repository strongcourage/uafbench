#!/bin/sh 
#
# recsel.sh - System tests for recsel.
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
: ${crypt_support=yes}

. $srcdir/testutils.sh
test_init "recsel"

#
# Create input files.
#

test_declare_input_file empty-file ''

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

test_declare_input_file multiline \
'field1: foo bar \
baz
field2: foo
+ bar
+ baz

field1: jo ja \
ju
field2:
+ foo
+ bar
'

test_declare_input_file repeated-fields \
'field1: value11
field2: value121
field2: value122
field3: value13

field1: value21
field2: value221
field2: value222
field3: value23
'

test_declare_input_file multiple-types \
'%rec: type1

field1: value11
field2: value12
field3: value13

%rec: type2

field1: value21
field2: value22
field3: value23

%rec: type3

field1: value31
field2: value32
field3: value33
'

test_declare_input_file integer-fields \
'field1: 314

field1: 10

field1: -10

field1: 0
'

test_declare_input_file real-fields \
'field1: 3.14

field1: 10.0

field1: -10.0

field1: 0
'

test_declare_input_file recurrent-fields \
'field1: value11
field2: value121
field2: value122

field2: value22
field3: value23
'

test_declare_input_file recurrent-fields-2 \
'Name: foo
Index: 5
Index: 9

Name: bar
Index: 7
'

test_declare_input_file dates \
'Date: Tue Nov 30 12:00:00 CET 2002

Date: Tue Nov 30 12:00:00 CET 2010

Date: Tue Nov 30 12:00:00 CET 2030
'

test_declare_input_file academy \
'Name: John Smith
Role: Professor
Age: 52

Name: Tom Johnson
Role: Professor
Age: 67

Name: Tommy Junior
Role: Student
Age: 5

Name: Johnny NotSoJunior
Role: Student
Age: 15
'

test_declare_input_file confidential \
'%rec: Account
%confidential: Password

User: foo
Password: encrypted-MHyd3Dqz+iaViL8h1m18sA==
'

test_declare_input_file sort \
'%rec: Sorted
%sort: Id
%type: Id int

Id: 100
Key: foo

Id: 20
Key: bar

Id: -2
Key: baz
'

test_declare_input_file sort-with-comment \
'%rec: Sorted
%sort: Id
%type: Id int

# Foo

# Bar

Id: 100
Key: foo

Id: 20
Key: bar

Id: -2
Key: baz
'

test_declare_input_file sort-multiple \
'%rec: SortMultiple
%sort: Class Price
%type: Price real

Item: one
Class: B
Price: 30

Item: two
Class: A
Price: 30

Item: three
Class: C
Price: 40

Item: four
Class: A
Price: 10

Item: five
Class: B
Price: 15
'

test_declare_input_file empty-field-values \
'a: a1
b:
c: 
d: d1
'

test_declare_input_file duplicated-fields \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value21
field1: value21

field1: value31
field2:
field2:
field3: value33

field1: foo
field1: foo
'

test_declare_input_file group-records \
'id: 3
pos: 1

id: 1
pos: 2

id: 1
pos: 3

id: 2
pos: 4

id: 3
pos: 5
'

test_declare_input_file group-records-missing \
'id: 3
pos: 1

id: 1
pos: 2

pos: 3

id: 2
pos: 4

id: 3
pos: 5
'

test_declare_input_file sales \
'Item: A
Date: 21 April 2012
Cost: 23

Item: B
Date: 20 April 2012
Cost: 50

Item: A
Date: 21 April 2012
Cost: 10

Item: C
Date: 12 January 2003
Cost: 12

Item: D
Date: 20 April 2012
Cost: 100
'

test_declare_input_file packages-maintainers \
'%rec: Package
%type: Maintainer,PreviousMaintainer rec Hacker

Name: GNU PDF
URL: http://www.gnupdf.org
Maintainer: jemarch@gnu.org
Maintainer: aleksander@gnu.org

Name: GNU recutils
URL: http://www.gnu.org/software/recutils
Maintainer: jemarch@gnu.org

Name: GNU Emacs
URL: http://www.gnu.org/software/emacs
Maintainer: stefan@monnier.com
PreviousMaintainer: rms@gnu.org

%rec: Hacker
%key: Email
%type: Email email

Name: Jose E. Marchesi
Email: jemarch@gnu.org
URL: http://www.jemarch.net

Name: Aleksander Morgado
Email: aleksander@gnu.org

Name: Richard M. Stallman
Email: rms@gnu.org
URL: http://www.stallman.org

Name: Stefan Monnier
Email: stefan@monnier.com
'

test_declare_input_file implications \
'%rec: Task

Id: 0
Status: Closed
ClosedBy: jemarch

Id: 1
Status: Closed

Id: 2
Status: Open
'

test_declare_input_file non-existant-foreign-key \
'%rec: Record
%type: Foreign rec NonExistant

Name: Fred
Foreign: xxx
'

test_declare_input_file several-foreign-keys \
'%rec: R
%key: Id

Id: R1
Description: this is R1

Id: R2
Description: this is R1

Id: R3
Description: this is R1

%rec: T
%key: Id
%type: Requirement rec R

Id: T1
Requirement: R1

Id: T2
Requirement: R2
Requirement: R3

Id: T3
Requirement: R3
'

test_declare_input_file unquoted-lisp-strings \
'foo: fo\o
bar: a quote"etouq a
'

test_declare_input_file single-char-field-names \
'a: 10
b: 20

c: 30
d: 40
'

test_declare_input_file record-with-comments \
'foo1: v1
# bar
foo2: v2

# jorl
bar1: v1
bar2: v2
# jurl
'

test_declare_input_file ignored-first-blanks \
'foo: v1
bar:	v2
'

test_declare_input_file negative_fields \
'foo: 10
bar: -10

foo: 9
bar: -9
'

#
# Declare tests
#

# Select the whole record set.
test_tool recsel-all-fields ok \
          recsel \
          '' \
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

# Print just one field. 
test_tool recsel-one-field ok \
          recsel \
          '-p field2' \
          multiple-records \
'field2: value12

field2: value22

field2: value32
'

# Print two fields.
test_tool recsel-two-fields ok \
          recsel \
          '-p field2,field3' \
          multiple-records \
'field2: value12
field3: value13

field2: value22
field3: value23

field2: value32
field3: value33
'

# Print values
test_tool recsel-values ok \
          recsel \
          '-P field2' \
          multiple-records \
'value12

value22

value32
'
test_tool recsel-values-long ok \
          recsel \
          '--print-values=field2' \
          multiple-records \
'value12

value22

value32
'

# Print multiple values
test_tool recsel-multiple-values ok \
          recsel \
          '-P field1,field3' \
          multiple-records \
'value11
value13

value21
value23

value31
value33
'

# Print collapsed
test_tool recsel-collapsed ok \
          recsel \
          '-C -p field1' \
          multiple-records \
'field1: value11
field1: value21
field1: value31
'

# Print values collapsed
test_tool recsel-values-collapsed ok \
          recsel \
          '-C -P field2,field3' \
          multiple-records \
'value12
value13
value22
value23
value32
value33
'

# Print in a row
test_tool recsel-row ok \
          recsel \
          '-R field1,field2' \
          multiple-records \
'value11 value12

value21 value22

value31 value32
'

test_tool recsel-row-long ok \
          recsel \
          '--print-row=field1,field2' \
          multiple-records \
'value11 value12

value21 value22

value31 value32
'

# Print collapsed in a row
test_tool recsel-collapsed-row ok \
          recsel \
          '-C -R field1,field2,field3' \
          multiple-records \
'value11 value12 value13
value21 value22 value23
value31 value32 value33
'

# Print all fields (multiline)
test_tool recsel-multi-all-fields ok \
          recsel \
          '' \
          multiline \
'field1: foo bar baz
field2: foo
+ bar
+ baz

field1: jo ja ju
field2: 
+ foo
+ bar
'

# Print values (multiline)
test_tool recsel-multi-values ok \
          recsel \
          '-P field1,field2' \
          multiline \
'foo bar baz
foo
bar
baz

jo ja ju

foo
bar
'
# Print collapsed values (multiline)
test_tool recsel-multi-collapsed-values ok \
          recsel \
          '-C -P field1,field2' \
          multiline \
'foo bar baz
foo
bar
baz
jo ja ju

foo
bar
'

# Print a count of all the records.
test_tool recsel-count ok \
          recsel \
          '-c' \
          multiple-records \
'3
'

# Subscripts.
test_tool recsel-subs ok \
          recsel \
          '-p field2[0]' \
          repeated-fields \
'field2: value121

field2: value221
'

test_tool recsel-subs-2 ok \
          recsel \
          '-p field2[1]' \
          repeated-fields \
'field2: value122

field2: value222
'

# Print records identified by its position into the record set.

test_tool recsel-index-first ok \
          recsel \
          '-n 0' \
          multiple-records \
'field1: value11
field2: value12
field3: value13
'

test_tool recsel-index-long ok \
          recsel \
          '--number=0' \
          multiple-records \
'field1: value11
field2: value12
field3: value13
'

test_tool recsel-index-second ok \
          recsel \
          '-n 1' \
          multiple-records \
'field1: value21
field2: value22
field3: value23
'

test_tool recsel-index-toobig ok \
          recsel \
          '-n 100' \
          multiple-records \
''

test_tool recsel-index-several ok \
          recsel \
          '-n 0,2' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value31
field2: value32
field3: value33
'

test_tool recsel-index-range-1 ok \
          recsel \
          '-n 0-1' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value22
field3: value23
'

test_tool recsel-index-range-2 ok \
          recsel \
          '-n 1-2' \
          multiple-records \
'field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33
'

test_tool recsel-index-range-3 ok \
          recsel \
          '-n 0,1-2' \
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

test_tool recsel-index-invalid-1 xfail \
          recsel \
          '-n 0,' \
          multiple-records

test_tool recsel-index-invalid-2 xfail \
          recsel \
          '-n ,0' \
          multiple-records

test_tool recsel-index-invalid-3 xfail \
          recsel \
          '-n 0,1,2-' \
          multiple-records

test_tool recsel-index-invalid-4 xfail \
          recsel \
          '-n 0,1,,2' \
          multiple-records

# Print records of several types.

test_tool recsel-type ok \
          recsel \
          '-t type1' \
          multiple-types \
'field1: value11
field2: value12
field3: value13
'

test_tool recsel-type-2 ok \
          recsel \
          '-t type2' \
          multiple-types \
'field1: value21
field2: value22
field3: value23
'

test_tool recsel-type-3 ok \
          recsel \
          '-t type2' \
          multiple-types \
'field1: value21
field2: value22
field3: value23
'

test_tool recsel-type-4 ok \
          recsel \
          '-t type3' \
          multiple-types \
'field1: value31
field2: value32
field3: value33
'

# Selection expressions.

test_tool recsel-sex-integer-equal ok \
          recsel \
          '-e "field1 = 0"' \
          integer-fields \
'field1: 0
'

test_tool recsel-sex-integer-equal-hex ok \
          recsel \
          '-e "field1 = -0xa"' \
          integer-fields \
'field1: -10
'

test_tool recsel-sex-integer-equal-oct ok \
          recsel \
          '-e "field1 = -012"' \
          integer-fields \
'field1: -10
'

test_tool recsel-sex-integer-nonequal ok \
          recsel \
          '-e "field1 != 314"' \
          integer-fields \
'field1: 10

field1: -10

field1: 0
'

test_tool recsel-sex-integer-lessthan ok \
          recsel \
          '-e "field1 < -5"' \
          integer-fields \
'field1: -10
'

test_tool recsel-sex-integer-lessequalthan ok \
          recsel \
          '-e "field1 <= 0"' \
          integer-fields \
'field1: -10

field1: 0
'

test_tool recsel-sex-integer-biggerthan ok \
          recsel \
          '-e "field1 > 10"' \
          integer-fields \
'field1: 314
'

test_tool recsel-sex-integer-biggerequalthan ok \
          recsel \
          '-e "field1 >= 10"' \
          integer-fields \
'field1: 314

field1: 10
'

test_tool recsel-sex-integer-plus ok \
          recsel \
          '-e "field1 + 2 = 316"' \
          integer-fields \
'field1: 314
'

test_tool recsel-sex-integer-minus ok \
          recsel \
          '-e "field1 - 2 = -12"' \
          integer-fields \
'field1: -10
'

test_tool recsel-sex-integer-mul ok \
          recsel \
          '-e "field1 * 20 = 200"' \
          integer-fields \
'field1: 10
'

test_tool recsel-sex-integer-div ok \
          recsel \
          '-e "field1 / 2 = 5"' \
          integer-fields \
'field1: 10
'

test_tool recsel-sex-integer-mod ok \
          recsel \
          '-e "field1 % 313 = 1"' \
          integer-fields \
'field1: 314
'

test_tool recsel-sex-integer-not ok \
          recsel \
          '-e "!field1"' \
          integer-fields \
'field1: 0
'

test_tool recsel-sex-integer-and ok \
          recsel \
          '-e "field1 && field1"' \
          integer-fields \
'field1: 314

field1: 10

field1: -10
'

test_tool recsel-sex-integer-or ok \
          recsel \
          '-e "field1 || 1"' \
          integer-fields \
'field1: 314

field1: 10

field1: -10

field1: 0
'

test_tool recsel-sex-real-equal ok \
          recsel \
          '-e "field1 = 3.14"' \
          real-fields \
'field1: 3.14
'

test_tool recsel-sex-real-nonequal ok \
          recsel \
          '-e "field1 != 3.14"' \
          real-fields \
'field1: 10.0

field1: -10.0

field1: 0
'

test_tool recsel-sex-real-lessthan ok \
          recsel \
          '-e "field1 < -5.2"' \
          real-fields \
'field1: -10.0
'
test_tool recsel-sex-real-biggerthan ok \
          recsel \
          '-e "field1 > 3.14"' \
          real-fields \
'field1: 10.0
'

test_tool recsel-sex-real-plus ok \
          recsel \
          '-e "((field1 + 2) > 5.14) && ((field1 + 2) < 5.15)"' \
          real-fields \
'field1: 3.14
'

#test_tool recsel-sex-real-minus \
#          recsel \
#          '-e "((field1 - 2.0) > -12.0)"' \
#          real-fields \
#'field1: -10.0
#'

test_tool recsel-sex-real-mul ok \
          recsel \
          '-e "field1 * 20.0 = 200.0"' \
          real-fields \
'field1: 10.0
'

test_tool recsel-sex-sharp-zero ok \
          recsel \
          '-e "#field1 = 0"' \
          recurrent-fields \
'field2: value22
field3: value23
'

test_tool recsel-sex-sharp-one ok \
          recsel \
          '-e "#field3 = 1"' \
          recurrent-fields \
'field2: value22
field3: value23
'

test_tool recsel-sex-sharp-multiple ok \
          recsel \
          '-e "#field2 = 2"' \
          recurrent-fields \
'field1: value11
field2: value121
field2: value122
'

test_tool recsel-sex-sharp-multiple-2 ok \
          recsel \
          '-e "#Index = 1"' \
          recurrent-fields-2 \
'Name: bar
Index: 7
'

test_tool recsel-sex-sharp-subname ok \
          recsel \
          '-t Package -j Maintainer -e "#Maintainer.URL" -P Maintainer.URL' \
          packages-maintainers \
'http://www.jemarch.net

http://www.jemarch.net
'

test_tool recsel-sex-match ok \
          recsel \
          '-p Name -e "Name ~ '\''Tom'\''"' \
          academy \
'Name: Tom Johnson

Name: Tommy Junior
'

test_tool recsel-sex-date-sametime ok \
          recsel \
          '-e "Date == '\''Tue Nov 30 12:00:00 CET 2010'\''"' \
          dates \
'Date: Tue Nov 30 12:00:00 CET 2010
'

test_tool recsel-sex-date-before ok \
          recsel \
          '-e "Date << '\''Tue Nov 30 12:00:00 CET 2030'\''"' \
          dates \
'Date: Tue Nov 30 12:00:00 CET 2002

Date: Tue Nov 30 12:00:00 CET 2010
'

test_tool recsel-sex-date-after ok \
          recsel \
          '-e "Date >> '\''Tue Nov 30 12:00:00 CET 2002'\''"' \
          dates \
'Date: Tue Nov 30 12:00:00 CET 2010

Date: Tue Nov 30 12:00:00 CET 2030
'

test_tool recsel-sex-implies-1 ok \
          recsel \
          '-e "Status = '\''Closed'\'' => #ClosedBy"' \
          implications \
'Id: 0
Status: Closed
ClosedBy: jemarch

Id: 2
Status: Open
'

test_tool recsel-sex-conditional-1 ok \
          recsel \
          '-e "Role ~ '\''Professor'\'' ? Age > 65 : Age < 10" -p Name' \
          academy \
'Name: Tom Johnson

Name: Tommy Junior
'

test_tool recsel-sex-conditional-2 ok \
          recsel \
          '-e "(Role ~ '\''Professor'\'') ? (Age < 65) : (Age > 10)" -p Name' \
          academy \
'Name: John Smith

Name: Johnny NotSoJunior
'

test_tool recsel-sex-string-single-quote ok \
          recsel \
          '-e "(Role ~ '\''Professor'\'')" -p Name' \
          academy \
'Name: John Smith

Name: Tom Johnson
'

test_tool recsel-sex-string-double-quote ok \
          recsel \
          "-e '(Role ~ "\""Professor"\"")' -p Name" \
          academy \
'Name: John Smith

Name: Tom Johnson
'

test_tool recsel-sex-string-multiline ok \
          recsel \
          "-e 'field2 = "\""
foo
bar"\""' -c" \
          multiline \
'1
'

test_tool recsel-sex-string-equal ok \
          recsel \
          "-e 'field2 = \"value12\"'" \
          multiple-records \
'field1: value11
field2: value12
field3: value13
'

test_tool recsel-sex-string-non-equal ok \
          recsel \
          "-e 'field2 != \"value12\"'" \
          multiple-records \
'field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33
'

test_tool recsel-sex-string-equal-insensitive ok \
          recsel \
          "-i -e 'field2 = \"vaLue12\"'" \
          multiple-records \
'field1: value11
field2: value12
field3: value13
'

test_tool recsel-sex-string-non-equal-insensitive ok \
          recsel \
          "-i -e 'field2 != \"vaLue12\"'" \
          multiple-records \
'field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33
'

test_tool recsel-quick-simple ok \
          recsel \
          "-q value22" \
          multiple-records \
'field1: value21
field2: value22
field3: value23
'

test_tool recsel-quick-not-found ok \
          recsel \
          "-q notfound" \
          multiple-records \
''

test_tool recsel-quick-and-sex xfail \
          recsel \
          "-q foo -e 'Bar = 10'" \
          multiple-records

test_tool recsel-sex-and-quick xfail \
          recsel \
          "-e 'Bar = 10' -q foo" \
          multiple-records

test_tool recsel-quick-and-num xfail \
          recsel \
          "-q foo -n 5" \
          multiple-records

test_tool recsel-num-and-quick xfail \
          recsel \
          "-n 5 -q foo" \
          multiple-records

test_tool recsel-num-and-random xfail \
          recsel \
          "-n 5 -m 2" \
          multiple-records

test_tool recsel-sex-and-random xfail \
          recsel \
          "-e 'field1 = 10' -m 2" \
          multiple-records

test_tool recsel-quick-and-random xfail \
          recsel \
          "-q foo -m 1" \
          multiple-records

if test "x$crypt_support" = "xyes"; then

    test_tool recsel-confidential ok \
              recsel \
              '-s secret' \
              confidential \
'User: foo
Password: secret
'

test_tool recsel-confidential-fex ok \
          recsel \
          '-s secret -p Password' \
          confidential \
'Password: secret
'

test_tool recsel-confidential-fex-value ok \
          recsel \
          '-s secret -P Password' \
          confidential \
'secret
'

test_tool recsel-confidential-num ok \
          recsel \
          '-s secret -n 0' \
          confidential \
'User: foo
Password: secret
'

fi # crypt_support

test_tool recsel-sort ok \
          recsel \
          '' \
          sort \
'Id: -2
Key: baz

Id: 20
Key: bar

Id: 100
Key: foo
'

test_tool recsel-sort-with-comment ok \
          recsel \
          '' \
          sort-with-comment \
'Id: -2
Key: baz

Id: 20
Key: bar

Id: 100
Key: foo
'

test_tool recsel-sort-multiple ok \
          recsel \
          '' \
          sort-multiple \
'Item: four
Class: A
Price: 10

Item: two
Class: A
Price: 30

Item: five
Class: B
Price: 15

Item: one
Class: B
Price: 30

Item: three
Class: C
Price: 40
'

test_tool recsel-sort-field ok \
          recsel \
          '-S Key' \
          sort \
'Id: 20
Key: bar

Id: -2
Key: baz

Id: 100
Key: foo
'

test_tool recsel-sort-field-multiple ok \
          recsel \
          '-S Price,Class' \
          sort-multiple \
'Item: four
Class: A
Price: 10

Item: five
Class: B
Price: 15

Item: two
Class: A
Price: 30

Item: one
Class: B
Price: 30

Item: three
Class: C
Price: 40
'
test_tool recsel-sort-field-nonexist ok \
          recsel \
          '-S doesnotexist' \
          sort \
'Id: 100
Key: foo

Id: 20
Key: bar

Id: -2
Key: baz
'

test_tool recsel-empty-field-values ok \
          recsel \
          '' \
          empty-field-values \
'a: a1
b:
c:
d: d1
'

test_tool recsel-uniq ok \
          recsel \
          '-U' \
          duplicated-fields \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value21

field1: value31
field2:
field3: value33

field1: foo
'

test_tool recsel-uniq-long ok \
          recsel \
          '--uniq' \
          duplicated-fields \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value21

field1: value31
field2:
field3: value33

field1: foo
'

test_tool recsel-random-all ok \
          recsel \
          '-m 0' \
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

test_tool recsel-random-one ok \
          recsel \
          '-m 1' \
          one-record \
'field1: value1
field2: value2
field3: value3
'

test_tool recsel-group-records ok \
          recsel \
          '-G id' \
          group-records \
'id: 1
pos: 2
pos: 3

id: 2
pos: 4

id: 3
pos: 1
pos: 5
'

test_tool recsel-group-records-sort ok \
          recsel \
          '-G pos' \
          group-records \
'id: 3
pos: 1

id: 1
pos: 2

id: 1
pos: 3

id: 2
pos: 4

id: 3
pos: 5
'

test_tool recsel-group-records-non-existing-field ok \
          recsel \
          '-G doesnotexist' \
          group-records \
'id: 3
pos: 1

id: 1
pos: 2

id: 1
pos: 3

id: 2
pos: 4

id: 3
pos: 5
'

test_tool recsel-group-one-record ok \
          recsel \
          '-G field3' \
          one-record \
'field1: value1
field2: value2
field3: value3
'

test_tool recsel-group-records-missing ok \
          recsel \
          '-G id' \
          group-records-missing \
'pos: 3

id: 1
pos: 2

id: 2
pos: 4

id: 3
pos: 1
pos: 5
'

test_tool recsel-group-multiple-fields ok \
          recsel \
          '-G Date,Item' \
          sales \
'Item: C
Date: 12 January 2003
Cost: 12

Item: B
Date: 20 April 2012
Cost: 50

Item: D
Date: 20 April 2012
Cost: 100

Item: A
Date: 21 April 2012
Cost: 23
Cost: 10
'

# rewrite rule

test_tool recsel-aggregate-field-name ok \
           recsel \
           '-p "Count(Cost)"' \
           sales \
'Count_Cost: 5
'

test_tool recsel-aggregate-case-insensitive ok \
          recsel \
          '-p "cOuNt(Cost)"' \
          sales \
'cOuNt_Cost: 5
'

test_tool recsel-aggregate-rewrite-rule ok \
          recsel \
          '-p "Count(Cost):MyCount"' \
          sales \
'MyCount: 5
'

test_tool recsel-aggregate-count-overall ok \
          recsel \
          '-P "Count(Cost)"' \
          sales \
'5
'

test_tool recsel-aggregate-count-grouped ok \
          recsel \
          '-p "Item,Count(Cost)" -G Item' \
          sales \
'Item: A
Count_Cost: 2

Item: B
Count_Cost: 1

Item: C
Count_Cost: 1

Item: D
Count_Cost: 1
'

test_tool recsel-aggregate-avg-overall ok \
          recsel \
          '-P "Avg(Cost)"' \
          sales \
'39
'

test_tool recsel-aggregate-avg-grouped ok \
          recsel \
          '-p "Item,Avg(Cost)" -G Item' \
          sales \
'Item: A
Avg_Cost: 16.5

Item: B
Avg_Cost: 50

Item: C
Avg_Cost: 12

Item: D
Avg_Cost: 100
'

test_tool recsel-aggregate-sum-overall ok \
          recsel \
          '-P "Sum(Cost)"' \
          sales \
'195
'

test_tool recsel-aggregate-sum-grouped ok \
          recsel \
          '-p "Item,Sum(Cost):TotalCost" -G Item' \
          sales \
'Item: A
TotalCost: 33

Item: B
TotalCost: 50

Item: C
TotalCost: 12

Item: D
TotalCost: 100
'

test_tool recsel-aggregate-sum-negative ok \
          recsel \
          '-p "Sum(bar):TotalBar"' \
          negative_fields \
'TotalBar: -19
'

test_tool recsel-aggregate-min-overall ok \
          recsel \
          '-P "Min(Cost)"' \
          sales \
'10
'

test_tool recsel-aggregate-min-grouped ok \
          recsel \
          '-p "Item,Min(Cost)" -G Item' \
          sales \
'Item: A
Min_Cost: 10

Item: B
Min_Cost: 50

Item: C
Min_Cost: 12

Item: D
Min_Cost: 100
'

test_tool recsel-aggregate-max-overall ok \
          recsel \
          '-P "Max(Cost)"' \
          sales \
'100
'

test_tool recsel-aggregate-max-grouped ok \
          recsel \
          '-p "Item,Max(Cost)" -G Item' \
          sales \
'Item: A
Max_Cost: 23

Item: B
Max_Cost: 50

Item: C
Max_Cost: 12

Item: D
Max_Cost: 100
'

test_tool recsel-fex-rewrite-all ok \
          recsel \
          "-p field1,field2:xxx,field3" \
          repeated-fields \
'field1: value11
xxx: value121
xxx: value122
field3: value13

field1: value21
xxx: value221
xxx: value222
field3: value23
'

test_tool recsel-fex-rewrite-subscript-single ok \
          recsel \
          "-p field1,field2[0]:xxx,field2[1],field3" \
          repeated-fields \
'field1: value11
xxx: value121
field2: value122
field3: value13

field1: value21
xxx: value221
field2: value222
field3: value23
'
test_tool recsel-fex-rewrite-subscript-multi ok \
          recsel \
          "-p field1,field2[0-1]:xxx,field3" \
          repeated-fields \
'field1: value11
xxx: value121
xxx: value122
field3: value13

field1: value21
xxx: value221
xxx: value222
field3: value23
'

test_tool recsel-fex-rewrite-several ok \
          recsel \
          "-p field1,field2:xxx,field3:yyy" \
          repeated-fields \
'field1: value11
xxx: value121
xxx: value122
yyy: value13

field1: value21
xxx: value221
xxx: value222
yyy: value23
'

test_tool recsel-descriptor ok \
          recsel \
          "-t type2 -d" \
          multiple-types \
'%rec: type2

field1: value21
field2: value22
field3: value23
'

test_tool recsel-nonexistent-print ok \
          recsel \
          '-p dontexist' \
          multiple-records \
          ''

test_tool recsel-join-default-rset xfail \
          recsel \
          '-j field2' \
          multiple-records

test_tool recsel-join-no-foreign ok \
          recsel \
          '-t type1 -j field2' \
          multiple-types \
'field1: value11
field2: value12
field3: value13
'

test_tool recsel-non-existant-foreign-key ok \
          recsel \
          '-t Record -j Foreign' \
          non-existant-foreign-key \
'Name: Fred
Foreign: xxx
'

test_tool recsel-several-foreign-keys ok \
          recsel \
          '-t T -j Requirement -p Id,Requirement.Id' \
          several-foreign-keys \
'Id: T1
Requirement_Id: R1

Id: T2
Requirement_Id: R2

Id: T2
Requirement_Id: R3

Id: T3
Requirement_Id: R3
'

test_tool recsel-join-descriptor ok \
          recsel \
          '-t Package -j Maintainer -d -p Name' \
         packages-maintainers \
'%rec: Package_Maintainer

Name: GNU PDF

Name: GNU PDF

Name: GNU recutils

Name: GNU Emacs
'

test_tool recsel-unquoted-lisp-strings ok \
          recsel \
          '--print-sexps' \
          unquoted-lisp-strings \
'(record 0 (
(field 0 "foo" "fo\\o")
(field 11 "bar" "a quote\"etouq a")))
'
test_tool recsel-sex-single-char-field-names ok \
          recsel \
          '-e "c = 30"' \
          single-char-field-names \
'c: 30
d: 40
'

test_tool recsel-record-with-comments ok \
          recsel \
          '' \
          record-with-comments \
'foo1: v1
foo2: v2

bar1: v1
bar2: v2
'

test_tool recsel-ignored-first-blanks ok \
          recsel \
          '' \
          ignored-first-blanks \
'foo: v1
bar: v2
'

#
# Cleanup
#

test_cleanup
exit $?

# End of recsel.sh
