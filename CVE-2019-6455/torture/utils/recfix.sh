#!/bin/sh
#
# recfix.sh - System tests for recfix.
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
: ${builddir=.}
: ${crypt_support=yes}
: ${uuid_support=yes}

. $builddir/config.sh
. $srcdir/testutils.sh
test_init "recfix"

#
# Create input files.
#

test_declare_input_file type-rec-valid \
'%rec: Package
%type: Maintainer rec Hacker

Name: GNU recutils
Maintainer: jemarch@gnu.org
'

test_declare_input_file type-rec-invalid-empty \
'%rec: Package
%type: Maintainer rec
'

test_declare_input_file type-rec-invalid-malformed \
'%rec: Package
%type: Maintainer rec foo;invalid
'

test_declare_input_file type-int-valid \
'%rec: Types
%type: Integer int

Integer: 10

Integer: 0x10

Integer: 012

Integer: 0xaaaa0000

Integer: -0x10

Integer: -0xFF
' 

test_declare_input_file type-int-invalid \
'%rec: Types
%type: Integer int

Integer: aaa

Integer: 0x

Integer: --0xF
'

test_declare_input_file type-real-valid \
'%rec: Types
%type: Real real

Real: 3.14
'

test_declare_input_file type-real-invalid \
'%rec: Types
%type: Real real

Real: 3..14
'

test_declare_input_file duplicated-keys \
'%rec: Keys
%key: Id

Id: 0

Id: 1

Id: 2

Id: 2

Id: 3
'

test_declare_input_file missing-mandatory \
'%rec: Mandatory
%mandatory: ma

foo: bar

bar: baz
ma: foo
'

test_declare_input_file several-unique \
'%rec: Unique
%unique: Id

Id: 0

Id: 1
Id: 2

Id: 3
'

test_declare_input_file referred-type \
'%rec: One
%type: foo int

foo: 10

foo: 20

%rec: Two

One:foo: 20
'

test_declare_input_file hidden-type \
'%rec: One
%type: foo int

foo: 10

foo: 20

%rec: Two
%type: foo line

One:foo: foobar
'

test_declare_input_file ranges-ok \
'%rec: Foo
%type: bar range -10 10
%type: baz range 10

bar: -10
baz: 0

bar: 10
baz: 10

bar: 2
baz: 5
'

test_declare_input_file ranges-hex-ok \
'%rec: Foo
%type: bar range -0x10 0x10

bar: -16

bar: 0

bar: 5

bar: 0x10
'

test_declare_input_file ranges-hex-invalid \
'%rec: Foo
%type: bar range 0 0xFF

bar: 0

bar: 0x100
'

test_declare_input_file ranges-oct-ok \
'%rec: Foo
%type: bar range -010 010

bar: -8

bar: 0

bar: 8
'

test_declare_input_file ranges-oct-invalid \
'%rec: Foo
%type: bar range -010 010

bar: -8

bar: 0

bar: 9
'

test_declare_input_file ranges-xfail-1 \
'%rec: Foo
%type: bar range -10 10
%type: baz range 10

bar: -25
baz: 0
'

test_declare_input_file ranges-xfail-2 \
'%rec: Foo
%type: bar range -10 10
%type: baz range 10

bar: 2
baz: 11
'

test_declare_input_file multiple-rec \
'%rec: foo
%type: bar int
%rec: bar

bar: 10
'

test_declare_input_file enum-valid \
'%rec: foo
%type: bar enum
+ KEY1 (This is key 1)
+ KEY2 (This is key 2)
+ KEY3 (This is key 3)

bar: KEY1

bar: KEY2

bar: KEY3
'

test_declare_input_file enum-invalid-1 \
'%rec: foo
%type: bar enum
+ KEY1 (This is key 1)
+ KEY2 ((This is key 2)
+ KEY3 (This is key 3)

bar: KEY1

bar: KEY2

bar: KEY3
'

test_declare_input_file enum-invalid-2 \
'%rec: foo
%type: bar enum
+ KEY1 (This is key 1)
+ KEY2 (This is key 2))
+ KEY3 (This is key 3)

bar: KEY1

bar: KEY2

bar: KEY3
'

test_declare_input_file type-size-valid \
'%rec: foo
%type: bar size 10

bar:

bar: xxx

bar: 1 2 3
+ 4 55
'

test_declare_input_file type-size-valid-hex \
'%rec: foo
%type: bar size 0xa

bar:

bar: xxx

bar: 1 2 3
+ 4 55
'

test_declare_input_file type-size-valid-oct \
'%rec: foo
%type: bar size 012

bar:

bar: xxx

bar: 1 2 3
+ 4 55
'

test_declare_input_file type-size-invalid \
'%rec: foo
%type: bar size 2

bar:

bar: xxx

bar: 1 2 3 \
+ 4 55
'

test_declare_input_file type-size-invalid-negative \
'%rec: foo
%type: bar size -2
'

test_declare_input_file prohibited-fields-ok \
'%rec: foo
%prohibit: banned

foo: bar

bar: baz
'

test_declare_input_file prohibited-fields \
'%rec: foo
%prohibit: banned1 banned2

foo: bar
banned1: foo

bar: baz
banned2: bar

bar: foo
'

test_declare_input_file auto-int \
'%rec: foo
%type: myint int
%auto: myint
'

test_declare_input_file auto-range \
'%rec: foo
%type: myrange range 0 10
%auto: myrange
'

test_declare_input_file auto-date \
'%rec: foo
%type: mydate date
%auto: mydate
'

test_declare_input_file auto-notype \
'%rec: foo
%auto: myint
'

test_declare_input_file auto-invalid-type \
'%rec: foo
%auto: key
%type: key line
'

test_declare_input_file auto-nofex \
'%rec: foo
%auto: this%is#not%a^ fex
'

test_declare_input_file size-invalid-1 \
'%rec: foo
%size: >
'

test_declare_input_file size-invalid-2 \
'%rec: foo
%size: foo
'

test_declare_input_file size-exact-zero \
'%rec: foo
%size: 0
'

test_declare_input_file size-exact-zero-invalid \
'%rec: foo
%size: 0

foo: bar
'

test_declare_input_file size-exact \
'%rec: foo
%size: 2

foo: bar

bar: baz
'

test_declare_input_file size-exact-hex \
'%rec: foo
%size: 0x2

foo: bar

bar: baz
'

test_declare_input_file size-exact-oct \
'%rec: foo
%size: 02

foo: bar

bar: baz
'

test_declare_input_file size-exact-invalid \
'%rec: foo
%size: 2

foo: bar
'

test_declare_input_file size-less \
'%rec: foo
%size: < 2

foo: bar
'

test_declare_input_file size-less-invalid \
'%rec: foo
%size: < 2

foo: bar

bar: baz
'

test_declare_input_file size-less-equal \
'%rec: foo
%size: <= 2

foo: bar

bar: baz
'

test_declare_input_file size-less-equal-invalid \
'%rec: foo
%size: <= 1

foo: bar

bar: baz
'

test_declare_input_file size-bigger \
'%rec: foo
%size: > 1

foo: bar

bar: baz
'

test_declare_input_file size-bigger-invalid \
'%rec: foo
%size: > 2

foo: bar
'

test_declare_input_file size-bigger-equal \
'%rec: foo
%size: >= 2

foo: bar

bar: baz
'

test_declare_input_file size-bigger-equal-invalid \
'%rec: foo
%size: >= 2

foo: bar
'

test_declare_input_file size-several \
'%rec: foo
%size: 2
%size: < 10
'

test_declare_input_file typedef-valid \
'%rec: foo
%typedef: Id_t int
%type: Id Id_t

Id: 10
'

test_declare_input_file typedef-valid-xfail \
'%rec: foo
%typedef: Id_t int
%type: Id Id_t

Id: xx
'

test_declare_input_file typedef-valid-with-blanks \
'%rec: foo
%typedef:     
+ Id_t int
%type: Id Id_t    
+    

Id: 10
'

test_declare_input_file typedef-valid-with-blanks-xfail \
'%rec: foo
%typedef:     
+ Id_t int
%type: Id Id_t    
+    

Id: xx
'

test_declare_input_file typedef-valid-order \
'%rec: foo
%type: Id Id_t
%typedef: Id_t int

Id: 10
'

test_declare_input_file typedef-valid-order-xfail \
'%rec: foo
%type: Id Id_t
%typedef: Id_t int

Id: xx
'

test_declare_input_file typedef-valid-chain \
'%rec: foo
%typedef: Foo_t Bar_t
%typedef: Bar_t Baz_t
%typedef: Baz_t int
%type: Foo Foo_t

Foo: 10
'

test_declare_input_file typedef-valid-chain-xfail \
'%rec: foo
%typedef: Foo_t Bar_t
%typedef: Bar_t Baz_t
%typedef: Baz_t int
%type: Foo Foo_t

Foo: xx
'

test_declare_input_file typedef-valid-multiple \
'%rec: foo
%typedef: Foo_t int
%typedef: Foo_t email
%type: Foo Foo_t

Foo: foo@bar.baz
'

test_declare_input_file typedef-valid-multiple-xfail \
'%rec: foo
%typedef: Foo_t int
%typedef: Foo_t email
%type: Foo Foo_t

Foo: 10
'

test_declare_input_file typedef-invalid-bad-type \
'%rec: foo
%typedef: Id_t int invalid
%type: Id Id_t

Id: 10
'

test_declare_input_file typedef-invalid-chain-undefined \
'%rec: foo
%typedef: Id_t Undefined_t
%type: Id Id_t

Id: 10
'

test_declare_input_file typedef-invalid-chain-loop \
'%rec: foo
%typedef: Foo_t Bar_t
%typedef: Bar_t Baz_t
%typedef: Baz_t Foo_t
%type: Id Foo_t

Id: 10
'

test_declare_input_file sortcheck-valid \
'%rec: foo
%sort: AField
'

test_declare_input_file sortcheck-with-blanks \
'%rec: foo
%sort:       
+       AField \
   
'
test_declare_input_file sortcheck-invalid-empty \
'%rec: foo
%sort:
'

test_declare_input_file sortcheck-invalid-field-name \
'%rec: foo
%sort: A/Field
'

test_declare_input_file sortcheck-with-several-fields-invalid \
'%rec: foo
%sort: BField
%sort: AField
'

test_declare_input_file unsorted-int \
'%rec: foo
%type: Id int
%sort: Id

Id: 4
Name: A Field

Id: 2
Name: C Field

Id: 1
Name: D Field

Id: 3
Name: B Field
'

test_declare_input_file unsorted-int-with-equals \
'%rec: foo
%type: Id int
%sort: Id

Id: 4
Name: A Field

Id: 2
Name: C Field

Id: 1
Name: D Field

Id: 3
Name: B Field

Id: 2
Name: Cbis Field
'

test_declare_input_file unsorted-range \
'%rec: foo
%type: Id range 0 10
%sort: Id

Id: 4
Name: A Field

Id: 2
Name: C Field

Id: 1
Name: D Field

Id: 3
Name: B Field
'

test_declare_input_file unsorted-range-with-equals \
'%rec: foo
%type: Id range 0 10
%sort: Id

Id: 4
Name: A Field

Id: 2
Name: C Field

Id: 1
Name: D Field

Id: 3
Name: B Field

Id: 2
Name: Cbis Field
'

test_declare_input_file unsorted-real \
'%rec: foo
%type: Id real
%sort: Id

Id: 4.2
Name: A Field

Id: 2.2
Name: C Field

Id: 1.2
Name: D Field

Id: 3.2
Name: B Field
'

test_declare_input_file unsorted-real-with-equals \
'%rec: foo
%type: Id real
%sort: Id

Id: 4.2
Name: A Field

Id: 2.2
Name: C Field

Id: 1.2
Name: D Field

Id: 3.2
Name: B Field

Id: 4.2
Name: Abis Field
'

test_declare_input_file unsorted-lex \
'%rec: foo
%sort: Name

Id: 4
Name: A Field

Id: 2
Name: C Field

Id: 1
Name: D Field

Id: 3
Name: B Field
'

test_declare_input_file unsorted-lex-with-equals \
'%rec: foo
%sort: Name

Id: 4
Name: A Field

Id: 2
Name: C Field

Id: 1
Name: D Field

Id: 3
Name: B Field

Id: 1
Name: Dbis Field
'

test_declare_input_file unsorted-bool \
'%rec: foo
%type: Bool bool
%sort: Bool

Id: 5
Bool: 1

Id: 1
Bool: 0

Id: 3
Bool: no

Id: 6
Bool: true

Id: 2
Bool: false

Id: 4
Bool: yes
'

test_declare_input_file unsorted-date \
'%rec: foo
%type: Date date
%sort: Date

Id: 1
Date: 24 September 1972

Id: 3
Date: 23 October 1972

Id: 5
Date: 26 May 1984

Id: 2
Date: 23 September 1972

Id: 4
Date: 1 April 1999
'

test_declare_input_file unsorted-date-with-equals \
'%rec: foo
%type: Date date
%sort: Date

Id: 1
Date: 24 September 1972

Id: 3
Date: 23 October 1972

Id: 5
Date: 26 May 1984

Id: 6
Date: 23 October 1972

Id: 2
Date: 23 September 1972

Id: 4
Date: 1 April 1999
'
test_declare_input_file unsorted-multiple \
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

test_declare_input_file confidential \
'%rec: foo
%confidential: Foo Bar Baz

Foo: encrypted-foo
Bar: encrypted-bar
Baz: encrypted-baz

%rec: bar

Foo: jorl
Bar: jarl
Baz: jerl
'

test_declare_input_file confidential-several \
'%rec: foo
%confidential: Foo Bar Baz
%confidential: Jorl
'

test_declare_input_file confidential-with-unencrypted-fields \
'%rec: foo
%confidential: Foo

Foo: encrypted-foo

Foo: Not encrypted
'

test_declare_input_file confidential-fields \
'%rec: foo
%confidential: Password WebPassword

User: user1
Password: secret1
Password: secret2
WebPassword: websecret1

User: user2
Password: secret2
Password: secret22
WebPassword: websecret2
'

test_declare_input_file encrypt \
'%rec: Account
%confidential: Secret

Secret: foo

Secret: bar

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: je
fo: fu

joo: ji
fo: ja
'

test_declare_input_file decrypt \
'%rec: Account
%confidential: Secret

Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Secret: encrypted-V1xOls6u5Zw/D5AOtZ9gfQ==

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: encrypted-MhsqXvDjqU9vOXG8QoHxKg==
fo: fu

joo: encrypted-By/F2HBy1wiim1fUWMVKRg==
fo: ja
'

test_declare_input_file encrypt-already-encrypted \
'%rec: Account
%confidential: Secret

Id: 1
Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Id: 2
Secret: bar
'

test_declare_input_file missing-auto-fields \
'%rec: Item
%auto: Id

Id: 0
Title: foo

Id: 1
Title: baz

Title: bar
'

test_declare_input_file uuid-fields-ok \
'%rec: Item
%type: Id uuid

Id: 550e8400-e29b-41d4-a716-446655440000
Name: Item 1

Id: 550e8401-e29b-41d4-a716-446655440000
Name: Item 2
'

test_declare_input_file uuid-fields-invalid \
'%rec: Item
%type: Id uuid

Id:
Name: Item 1

Id: foo
Name: Item 2
'

test_declare_input_file type-rec-norset \
'%rec: foo
%type: Foo rec bar

Foo: foo

Foo: bar
'

test_declare_input_file type-rec-nokey \
'%rec: foo
%type: Foo rec bar

Foo: foo

Foo: bar

%rec: bar

Bar: 10

Bar: 20
'

test_declare_input_file type-rec-key-notype \
'%rec: foo
%type: Foo rec bar

Foo: foo

Foo: bar

%rec: bar
%key: Bar

Bar: 10

Bar: 20
'

test_declare_input_file type-rec-key-type \
'%rec: foo
%type: Foo rec bar

Foo: 2

Foo: 3

%rec: bar
%key: Bar
%type: Bar int

Bar: 10

Bar: 20
'

test_declare_input_file type-rec-key-type-invalid \
'%rec: foo
%type: Foo rec bar

Foo: xxx

Foo: 3

%rec: bar
%key: Bar
%type: Bar int

Bar: 10

Bar: 20
'

test_declare_input_file constraint-sex-valid \
'%rec: Task
%constraint: Status = '\''Closed'\'' => #ClosedBy

Id: 0
Status: Closed
ClosedBy: jemarch

Id: 1
Status: Closed
ClosedBy: bar

Id: 2
Status: Open
'

test_declare_input_file constraint-sex-invalid \
'%rec: Task
%constraint: Status &= 'Closed' => #ClosedBy

Id: 0
Status: Closed
ClosedBy: jemarch

Id: 1
Status: Closed

Id: 2
Status: Open
'

test_declare_input_file constraint-sex-invalid-empty \
'%rec: Task
%constraint:

Id: 0
Status: Closed
ClosedBy: jemarch

Id: 1
Status: Closed

Id: 2
Status: Open
'

test_declare_input_file constraint-sex-with-violation \
'%rec: Task
%constraint: Status = '\''Closed'\''
+ => #ClosedBy = 0

Id: 0
Status: Closed
ClosedBy: jemarch

Id: 1
Status: Closed

Id: 2
Status: Open
'

test_declare_input_file constraint-sex-several-valid \
'%rec: Task
%constraint: Status = '\''Closed'\'' => #ClosedBy
%constraint: 1

Id: 0
Status: Closed
ClosedBy: jemarch

Id: 1
Status: Closed
ClosedBy: mr foo

Id: 2
Status: Open
'

test_declare_input_file constraint-sex-several-invalid \
'%rec: Task
%constraint: Status = '\''Closed'\'' => #ClosedBy
%constraint: Id = 1 => Status != '\''Closed'\''

Id: 0
Status: Closed
ClosedBy: jemarch

Id: 1
Status: Closed

Id: 2
Status: Open
'

test_declare_input_file unused-type \
'%rec: Task
%typedef: Status_t enum Open Closed
%key: Id

Id: 1
'

test_declare_input_file blank-line-after-record \
'foo: bar
  
'

test_declare_input_file blank-line-after-record-invalid \
'foo:bar
 x
'

test_declare_input_file hyphens-in-field-names \
'foo-bar: baz
'

test_declare_input_file ranges-min-ok \
'%rec: foo
%type: Negative range MIN -1

Negative: -1
Negative: -2147483648
'

test_declare_input_file ranges-min-invalid \
'%rec: foo
%type: Negative range MIN -1

Negative: -1
Negative: 0
'

test_declare_input_file ranges-max-ok \
'%rec: foo
%type: Positive range 0 MAX

Positive: 0
Positive: 2147483647
'

test_declare_input_file ranges-max-invalid \
'%rec: foo
%type: Positive range 0 MAX

Positive: -1
Positive: 2147483647
'

test_declare_input_file ranges-minmax-ok \
'%rec: foo
%type: Any range MIN MAX

Any: -2147483648
Any: 0
Any: 2147483647
'

test_declare_input_file ranges-minmax-invalid \
'%rec: foo
%type None range MAX MIN

None: -2147483648
None: 0
None: 2147483647
'

test_declare_input_file allowed-ok-1 \
'%rec: foo
%allowed: xxx

xxx: 10

xxx: 20
'

test_declare_input_file allowed-ok-2 \
'%rec: foo
%allowed: xxx yyy

xxx: 10

xxx: 20
yyy: 30
'

test_declare_input_file allowed-ok-3 \
'%rec: foo
%allowed: xxx
%allowed: yyy

xxx: 10

xxx: 20
yyy: 30
'

test_declare_input_file allowed-ok-4 \
'%rec: foo
%allowed: xxx
%allowed: yyy

xxx: 10

xxx: 20
yyy: 30
'

test_declare_input_file allowed-ok-5 \
'%rec: foo
%key: xkey
%mandatory: xmandatory
%allowed: xxx


xkey: jur
xxx: 10
xmandatory: jur

xxx: 20
xkey: jor
xmandatory: jur
'

test_declare_input_file allowed-xfail-1 \
'%rec: foo
%allowed: xxx

xxx: 10

xxx: 20
yyy: 30
'

test_declare_input_file allowed-xfail-2 \
'%rec: foo
%allowed: xxx yyy

xxx: 10

xxx: 20
zzz: 40
yyy: 30
'

test_declare_input_file allowed-xfail-3 \
'%rec: foo
%allowed: xxx
%allowed: yyy

xxx: 10

xxx: 20
zzz: 40
yyy: 30
'
                        
#
# Declare tests.
#

test_tool recfix-with-operation ok \
          recfix \
          '--check' \
          type-int-valid \
''

test_tool recfix-type-int-valid ok \
          recfix \
          '' \
          type-int-valid \
''

test_tool recfix-type-int-invalid xfail \
          recfix \
          '' \
          type-int-invalid

test_tool recfix-type-real-valid ok \
          recfix \
          '' \
          type-real-valid \
''

test_tool recfix-type-real-invalid xfail \
          recfix \
          '' \
          type-real-invalid

test_tool recfix-duplicated-keys xfail \
          recfix \
          '' \
          duplicated-keys

test_tool recfix-missing-mandatory xfail \
          recfix \
          '' \
          missing-mandatory

test_tool recfix-several-unique xfail \
          recfix \
          '' \
          several-unique

test_tool recfix-referred-type ok \
          recfix \
          '' \
          referred-type \
''

test_tool recfix-hidden-type ok \
          recfix \
          '' \
          hidden-type \
''

test_tool recfix-ranges-ok ok \
          recfix \
          '' \
          ranges-ok \
''

test_tool recfix-range-hex-ok ok \
          recfix \
          '' \
          ranges-hex-ok \
''

test_tool recfix-range-hex-xfail xfail \
          recfix \
          '' \
          ranges-hex-invalid

test_tool recfix-range-oct-ok ok \
          recfix \
          '' \
          ranges-oct-ok \
''

test_tool recfix-range-oct-xfail xfail \
          recfix \
          '' \
          ranges-oct-invalid

test_tool recfix-range-min-ok ok \
          recfix \
          '' \
          ranges-min-ok \
''

test_tool recfix-range-min-invalid xfail \
          recfix \
          '' \
          ranges-min-invalid

test_tool recfix-range-max-ok ok \
          recfix \
          '' \
          ranges-max-ok \
''

test_tool recfix-range-max-invalid xfail \
          recfix \
          '' \
          ranges-max-invalid

test_tool recfix-range-minmax-ok ok \
          recfix \
          '' \
          ranges-minmax-ok \
''

test_tool recfix-range-minmax-invalid xfail \
          recfix \
          '' \
          ranges-minmax-invalid

test_tool recfix-ranges-xfail-1 xfail \
          recfix \
          '' \
          ranges-xfail-1

test_tool recfix-ranges-xfail-2 xfail \
          recfix \
          '' \
          ranges-xfail-2

test_tool recfix-one-rec ok \
          recfix \
          '' \
          type-int-valid \
          ''

test_tool recfix-multiple-rec-in-descriptor xfail \
          recfix \
          '' \
          multiple-rec

test_tool recfix-enum-valid ok \
          recfix \
          '' \
          enum-valid \
          ''

test_tool recfix-enum-invalid-1 xfail \
          recfix \
          '' \
          enum-invalid-1

test_tool recfix-enum-invalid-2 xfail \
          recfix \
          '' \
          enum-invalid-2

test_tool recfix-type-size-valid ok \
          recfix \
          '' \
          type-size-valid \
''

test_tool recfix-type-size-valid-hex ok \
          recfix \
          '' \
          type-size-valid-hex \
''

test_tool recfix-type-size-valid-oct ok \
          recfix \
          '' \
          type-size-valid-oct \
''

test_tool recfix-type-size-invalid xfail \
          recfix \
          '' \
          type-size-invalid

test_tool recfix-type-size-invalid-negative xfail \
          recfix \
          '' \
          type-size-invalid-negative

test_tool recfix-prohibited-fields-ok ok \
          recfix \
          '' \
          prohibited-fields-ok \
''

test_tool recfix-prohibited-fields-fail xfail \
          recfix \
          '' \
          prohibited-fields

test_tool recfix-auto-int ok \
          recfix \
          '' \
          auto-int \
''

test_tool recfix-auto-range ok \
          recfix \
          '' \
          auto-range \
''

test_tool recfix-auto-date ok \
          recfix \
          '' \
          auto-date \
''

test_tool recfix-auto-notype ok \
          recfix \
          '' \
          auto-notype \
''

test_tool recfix-auto-invalid-type xfail \
          recfix \
          '' \
          auto-invalid-type

test_tool recfix-auto-nofex xfail \
          recfix \
          '' \
          auto-nofex

test_tool recfix-size-invalid-1 xfail \
          recfix \
          '' \
          size-invalid-1

test_tool recfix-size-invalid-2 xfail \
          recfix \
          '' \
          size-invalid-2

test_tool recfix-size-exact-zero ok \
          recfix \
          '' \
          size-exact-zero \
''

test_tool recfix-size-exact-zero-invalid xfail \
          recfix \
          '' \
          size-exact-zero-invalid

test_tool recfix-size-exact ok \
          recfix \
          '' \
          size-exact \
''

test_tool recfix-size-exact-hex ok \
          recfix \
          '' \
          size-exact-hex \
''

test_tool recfix-size-exact-oct ok \
          recfix \
          '' \
          size-exact-oct \
''

test_tool recfix-size-exact-invalid xfail \
          recfix \
          '' \
          size-exact-invalid

test_tool recfix-size-less ok \
          recfix \
          '' \
          size-less \
''

test_tool recfix-size-less-invalid xfail \
          recfix \
          '' \
          size-less-invalid

test_tool recfix-size-less-equal ok \
          recfix \
          '' \
          size-less-equal \
''

test_tool recfix-size-less-equal-invalid xfail \
          recfix \
          '' \
          size-less-equal-invalid

test_tool recfix-size-bigger ok \
          recfix \
          '' \
          size-bigger \
''

test_tool recfix-size-bigger-invalid xfail \
          recfix \
          '' \
          size-bigger-invalid

test_tool recfix-size-bigger-equal ok \
          recfix \
          '' \
          size-bigger-equal \
''

test_tool recfix-size-bigger-equal-invalid xfail \
          recfix \
          '' \
          size-bigger-equal-invalid

test_tool recfix-size-several xfail \
          recfix \
          '' \
          size-several

test_tool recfix-typedef-valid ok \
          recfix \
          '' \
          typedef-valid \
''

test_tool recfix-typedef-valid-xfail xfail \
          recfix \
          '' \
          typedef-valid-xfail

test_tool recfix-typedef-valid-with-blanks ok \
          recfix \
          '' \
          typedef-valid-with-blanks \
''

test_tool recfix-typedef-valid-with-blanks-xfail xfail \
          recfix \
          '' \
          typedef-valid-with-blanks-xfail

test_tool recfix-typedef-valid-order ok \
          recfix \
          '' \
          typedef-valid-order \
''

test_tool recfix-typedef-valid-order-xfail xfail \
          recfix \
          '' \
          typedef-valid-order-xfail

test_tool recfix-typedef-valid-chain ok \
          recfix \
          '' \
          typedef-valid-chain \
''

test_tool recfix-typedef-valid-chain-xfail xfail \
          recfix \
          '' \
          typedef-valid-chain-xfail

test_tool recfix-typedef-valid-multiple ok \
          recfix \
          '' \
          typedef-valid-multiple \
''

test_tool recfix-typedef-valid-multiple-xfail xfail \
          recfix \
          '' \
          typedef-valid-multiple-xfail

test_tool recfix-typedef-invalid-bad-type xfail \
          recfix \
          '' \
          typedef-invalid-bad-type

test_tool recfix-typedef-invalid-chain-undefined xfail \
          recfix \
          '' \
          typedef-invalid-chain-undefined

test_tool recfix-typedef-invalid-chain-loop xfail \
          recfix \
          '' \
          typedef-invalid-chain-loop

test_tool recfix-sortcheck-valid ok \
          recfix \
          '' \
          sortcheck-valid \
''

test_tool recfix-sortcheck-with-blanks ok \
          recfix \
          '' \
          sortcheck-with-blanks \
''

test_tool recfix-sortcheck-invalid-empty xfail \
          recfix \
          '' \
          sortcheck-invalid-empty

test_tool recfix-sortcheck-invalid-field-name xfail \
          recfix \
          '' \
          sortcheck-invalid-field-name

if test "x$crypt_support" = "yes"; then

test_tool recfix-confidential ok \
          recfix \
          '--check' \
          confidential \
          ''

test_tool recfix-confidential-several ok \
          recfix \
          '--check' \
          confidential-several \
          ''

test_tool recfix-confidential-with-unencrypted-fields xfail \
          recfix \
          '--check' \
          confidential-with-unencrypted-fields

test_tool recfix-encrypt ok \
          recfix \
          '--encrypt -s foo' \
          encrypt \
'%rec: Account
%confidential: Secret

Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Secret: encrypted-V1xOls6u5Zw/D5AOtZ9gfQ==

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: encrypted-MhsqXvDjqU9vOXG8QoHxKg==
fo: fu

joo: encrypted-By/F2HBy1wiim1fUWMVKRg==
fo: ja
'

test_tool recfix-encrypt-password-long ok \
          recfix \
          '--encrypt --password=foo' \
          encrypt \
'%rec: Account
%confidential: Secret

Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Secret: encrypted-V1xOls6u5Zw/D5AOtZ9gfQ==

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: encrypted-MhsqXvDjqU9vOXG8QoHxKg==
fo: fu

joo: encrypted-By/F2HBy1wiim1fUWMVKRg==
fo: ja
'

test_tool recfix-encrypt-without-password xfail \
          recfix \
          '--encrypt' \
          encrypt

test_tool recfix-encrypt-password-before-operation xfail \
          recfix \
          '-s foo --encrypt' \
          encrypt

test_tool recfix-encrypt-already-encrypted xfail \
          recfix \
          '--encrypt -s foo' \
          encrypt-already-encrypted

test_tool recfix-encrypt-already-encrypted-force ok \
          recfix \
          '--force --encrypt -s foo' \
          encrypt-already-encrypted \
'%rec: Account
%confidential: Secret

Id: 1
Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Id: 2
Secret: encrypted-V1xOls6u5Zw/D5AOtZ9gfQ==
'

test_tool recfix-decrypt ok \
          recfix \
          '--decrypt -s foo' \
          decrypt \
'%rec: Account
%confidential: Secret

Secret: foo

Secret: bar

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: je
fo: fu

joo: ji
fo: ja
'

test_tool recfix-decrypt-invalid-password ok \
          recfix \
          '--decrypt -s bar' \
          decrypt \
'%rec: Account
%confidential: Secret

Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Secret: encrypted-V1xOls6u5Zw/D5AOtZ9gfQ==

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: encrypted-MhsqXvDjqU9vOXG8QoHxKg==
fo: fu

joo: encrypted-By/F2HBy1wiim1fUWMVKRg==
fo: ja
'

fi # crypt_support

test_tool recfix-sort-several-fields-invalid xfail \
          recfix \
          '--check' \
          sortcheck-with-several-fields-invalid

test_tool recfix-sort-ints ok \
          recfix \
          '--sort' \
          unsorted-int \
'%rec: foo
%type: Id int
%sort: Id

Id: 1
Name: D Field

Id: 2
Name: C Field

Id: 3
Name: B Field

Id: 4
Name: A Field
'

test_tool recfix-sort-multiple ok \
          recfix \
          '--sort' \
          unsorted-multiple \
'%rec: SortMultiple
%sort: Class Price
%type: Price real

Item: four
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

test_tool recfix-sort-ints-with-equals ok \
          recfix \
          '--sort' \
          unsorted-int-with-equals \
'%rec: foo
%type: Id int
%sort: Id

Id: 1
Name: D Field

Id: 2
Name: C Field

Id: 2
Name: Cbis Field

Id: 3
Name: B Field

Id: 4
Name: A Field
'

test_tool recfix-sort-ranges ok \
          recfix \
          '--sort' \
          unsorted-range \
'%rec: foo
%type: Id range 0 10
%sort: Id

Id: 1
Name: D Field

Id: 2
Name: C Field

Id: 3
Name: B Field

Id: 4
Name: A Field
'

test_tool recfix-sort-ranges-with-equals ok \
          recfix \
          '--sort' \
          unsorted-range-with-equals \
'%rec: foo
%type: Id range 0 10
%sort: Id

Id: 1
Name: D Field

Id: 2
Name: C Field

Id: 2
Name: Cbis Field

Id: 3
Name: B Field

Id: 4
Name: A Field
'

test_tool recfix-sort-reals ok \
          recfix \
          '--sort' \
          unsorted-real \
'%rec: foo
%type: Id real
%sort: Id

Id: 1.2
Name: D Field

Id: 2.2
Name: C Field

Id: 3.2
Name: B Field

Id: 4.2
Name: A Field
'

test_tool recfix-sort-reals-with-equals ok \
          recfix \
          '--sort' \
          unsorted-real-with-equals \
'%rec: foo
%type: Id real
%sort: Id

Id: 1.2
Name: D Field

Id: 2.2
Name: C Field

Id: 3.2
Name: B Field

Id: 4.2
Name: A Field

Id: 4.2
Name: Abis Field
'

test_tool recfix-sort-lex ok \
          recfix \
          '--sort' \
          unsorted-lex \
'%rec: foo
%sort: Name

Id: 4
Name: A Field

Id: 3
Name: B Field

Id: 2
Name: C Field

Id: 1
Name: D Field
'

test_tool recfix-sort-lex-with-equals ok \
          recfix \
          '--sort' \
          unsorted-lex-with-equals \
'%rec: foo
%sort: Name

Id: 4
Name: A Field

Id: 3
Name: B Field

Id: 2
Name: C Field

Id: 1
Name: D Field

Id: 1
Name: Dbis Field
'

test_tool recfix-sort-dates ok \
          recfix \
          '--sort' \
          unsorted-date \
'%rec: foo
%type: Date date
%sort: Date

Id: 2
Date: 23 September 1972

Id: 1
Date: 24 September 1972

Id: 3
Date: 23 October 1972

Id: 5
Date: 26 May 1984

Id: 4
Date: 1 April 1999
'

test_tool recfix-sort-dates-with-equals ok \
          recfix \
          '--sort' \
          unsorted-date-with-equals \
'%rec: foo
%type: Date date
%sort: Date

Id: 2
Date: 23 September 1972

Id: 1
Date: 24 September 1972

Id: 3
Date: 23 October 1972

Id: 6
Date: 23 October 1972

Id: 5
Date: 26 May 1984

Id: 4
Date: 1 April 1999
'

test_tool recfix-sort-booleans ok \
          recfix \
          '--sort' \
          unsorted-bool \
'%rec: foo
%type: Bool bool
%sort: Bool

Id: 1
Bool: 0

Id: 3
Bool: no

Id: 2
Bool: false

Id: 5
Bool: 1

Id: 6
Bool: true

Id: 4
Bool: yes
'

test_tool recfix-missing-auto-fields ok \
          recfix \
          "--auto" \
          missing-auto-fields \
'%rec: Item
%auto: Id

Id: 0
Title: foo

Id: 1
Title: baz

Id: 2
Title: bar
'

if test "$uuid_support" = "yes"; then

test_tool recfix-uuid-ok ok \
          recfix \
          '--check' \
          uuid-fields-ok \
          ''

test_tool recfix-uuid-invalid xfail \
          recfix \
          '--check' \
          uuid-fields-invalid

fi

test_tool recfix-type-rec-valid ok \
          recfix \
          '--check' \
          type-rec-valid \
          ''

test_tool recfix-type-rec-invalid-empty xfail \
          recfix \
          '--check' \
          type-rec-invalid-empty

test_tool recfix-type-rec-invalid-malformed xfail \
          recfix \
          '--check' \
          type-rec-invalid-malformed

test_tool recfix-type-rec-norset ok \
          recfix \
          '--check' \
          type-rec-norset \
          ''
test_tool recfix-type-rec-nokey ok \
          recfix \
          '--check' \
          type-rec-nokey \
          ''

test_tool recfix-type-rec-key-notype ok \
          recfix \
          '--check' \
          type-rec-key-notype \
          ''

test_tool recfix-type-rec-key-type ok \
          recfix \
          '--check' \
          type-rec-key-type \
          ''

test_tool recfix-type-rec-key-type-invalid xfail \
          recfix \
          '--check' \
          type-rec-key-type-invalid

test_tool recfix-constraint-sex-valid ok \
          recfix \
          '--check' \
          constraint-sex-valid \
          ''

test_tool recfix-constraint-sex-invalid xfail \
          recfix \
          '--check' \
          constraint-sex-invalid

test_tool recfix-constraint-sex-invalid-empty xfail \
          recfix \
          '--check' \
          constraint-sex-invalid-empty

test_tool recfix-constraint-sex-several-valid ok \
          recfix \
          '--check' \
          constraint-sex-several-valid \
          ''

test_tool recfix-constraint-sex-several-invalid xfail \
          recfix \
          '--check' \
          constraint-sex-several-invalid

test_tool recfix-constraint-sex-with-violation xfail \
          recfix \
          '--check' \
          constraint-sex-with-violation

test_tool recfix-unused-type ok \
          recfix \
          '--check' \
          unused-type \
          ''

test_tool recfix-blank-line-after-record ok \
          recfix \
         '--check' \
         blank-line-after-record \
         ''

test_tool recfix-blank-line-after-record-invalid xfail \
          recfix \
         '--check' \
         blank-line-after-record-invalid

test_tool recfix-hyphens-in-field-names xfail \
          recfix \
          '--check' \
          hyphens-in-field-names

test_tool recfix-allowed-ok-1 ok \
          recfix \
          '--check' \
          allowed-ok-1 \
          ''

test_tool recfix-allowed-ok-2 ok \
          recfix \
          '--check' \
          allowed-ok-2 \
          ''

test_tool recfix-allowed-ok-3 ok \
          recfix \
          '--check' \
          allowed-ok-3 \
          ''

test_tool recfix-allowed-ok-4 ok \
          recfix \
          '--check' \
          allowed-ok-4 \
          ''

test_tool recfix-allowed-ok-5 ok \
          recfix \
          '--check' \
          allowed-ok-5 \
          ''

test_tool recfix-allowed-xfail-1 xfail \
          recfix \
          '--check' \
          allowed-xfail-1

test_tool recfix-allowed-xfail-2 xfail \
          recfix \
          '--check' \
          allowed-xfail-2

test_tool recfix-allowed-xfail-3 xfail \
          recfix \
          '--check' \
          allowed-xfail-3
        
#
# Cleanup.
#

test_cleanup
exit $?

# End of recfix.sh
