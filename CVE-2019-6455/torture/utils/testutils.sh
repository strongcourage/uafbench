#!/bin/sh
#
# testutils.sh - Misc utilities for testing the GNU recutils.
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

: ${builddir=.}
. $builddir/config.sh

# Create an input file.
# 
# $1 => Name of the file.
# $2 => Contents of the file.
test_declare_input_file ()
{
    # Check parameters.
    if test "$#" -ne "2"
    then
        echo "error: testutils: invalid parameters to test_declare_input_file"
        exit 1
    fi

    filename="${test_suite}-$1.in"
    contents="$2"

    # Create the input file.
    printf "%s" "$contents" > $filename

    # Add the file to the global list of temp files.
    test_tmpfiles="$test_tmpfiles $filename"
}

# Generate an input file.
#
# $1 => Name of the file.
# $2 => Number of records.
# $3 => Number of fields in each record.
test_gen_input_file ()
{
    # Check parameters.
    if test "$#" -ne "3"
    then
        echo "error: testutils: invalid parameters to test_gen_input_file"
        exit 1
    fi

    filename="$1.in"
    numrecords="$2"
    numfields="$3"

    # Create the contents of the file if it does not already exist.
    # This is important, since the generated test files will usually
    # be very big.

    echo -n "  Creating input file $filename ..."

    if test -f $filename; then
        echo " skipped"
    else
        recno=0
        while test "$recno" -lt "$numrecords"
        do
            fieldno=0
            while test "$fieldno" -lt "$numfields"
            do
                echo "field$fieldno: value$recno$fieldno" >> $filename
                fieldno=`expr $fieldno + 1`
            done
            echo "" >> $filename
            
            recno=`expr $recno + 1`
        done

        echo " done"
    fi
}

# Initialize the testing environment.
#
# $1 => Name of the test suite.
test_init ()
{
    if test "$#" -ne "1"; then
        echo "error: testutils: invalid parameters to test_init"
        exit 1
    fi

    test_suite=$1
    test_result="0"
    trap 'rm -fr $test_tmpfiles' EXIT 1 2 3 15

    echo "Running $test_suite test(s): "
}

# Delete temporary files and other cleanup.
#
# No parameters.
test_cleanup ()
{
    rm -fr $test_tmpfiles
    return $test_result
}

# Test a recutil.
#
# $1 => Test name.
# $2 => Expected result: ok, xfail, perf
# $3 => Utility to test.
# $4 => Parameters.
# $5 => Input file to use.
# $6 => Expected result.  Only for 'ok' tests.
test_tool ()
{
    # Check parameters.
    if test "$#" -lt "2" || \
       { test "$2" != "ok" && test "$2" != "xfail" && test "$2" != "perf"; } || \
       { test "$2" = "ok" && test "$#" -ne "6"; } || \
       { test "$2" = "perf" && test "$#" -ne "5"; } || \
       { test "$2" = "xfail" && test "$#" -ne "5"; }
    then
        echo "error: testutils: invalid parameters to test_tool"
        exit 1
    fi

    printf "  %s " $1

    status=$2
    utility=$3$EXEEXT
    parameters=$4
    input_file="${test_suite}-$5.in"
    ok_file="$1.ok"
    output_file="$1.out"
    error_file="$1.err"
    timing_file="$1.tim"
    postprocessed_output_file="$1.put"
    expected=$6

    test_tmpfiles="$test_tmpfiles $output_file $ok_file"
    if test "$status" = "perf"; then
        test_tmpfiles="$test_tmpfiles $timing_file"
    fi

    # Run the tool.
    if test "$status" = "perf"; then
        eval "cat $input_file | time -f %E -o $timing_file \
                  $utility $parameters > $output_file 2> $error_file"
    else
        eval "cat $input_file | $utility $parameters > $output_file 2> $error_file"
    fi
    res=$?

    if test "$status" = "ok" || test "$status" = "perf"
    then
        if test "$res" -ne "0"
        then
            printf "%s (see %s)\n" "error" "$error_file"
        else
            if test "$status" = "ok"; then
                # Check for the result in output_file.
                LC_ALL=C tr -d '\r' < $output_file > $postprocessed_output_file
                printf "%s" "$expected" > $ok_file
                cmp $ok_file $postprocessed_output_file > /dev/null 2>&1
                res=$?
                if test "$res" -eq "0"
                then
                    echo $status
                else
                    printf "%s (see %s)\n" "fail" "$1.diff"
                    diff $ok_file $postprocessed_output_file > $1.diff
                fi
                rm $error_file $postprocessed_output_file
            else
                elapsed_time=`cat $timing_file`
                echo $elapsed_time
                rm $error_file
            fi
        fi
    fi

    if test "$status" = "xfail"
    then
        if test "$res" -eq "0"
        then
            echo "error (expected failure)"
            res=1
        else
            echo $status
            
            # Don't accumulate any error.
            res=0
        fi

        rm $error_file
    fi

    # Accumulate the error.
    test_result=`expr $test_result + $res`
}

PATH=$srcdir:$PATH
export PATH

# End of testutils.sh
