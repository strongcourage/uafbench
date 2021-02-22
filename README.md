## UAF Fuzzing Benchmark

We create a fuzzing benchmark of Use-After-Free (UAF) and Double-Free (DF) bugs for our evaluations. It includes recent bugs found by existing (directed) greybox fuzzers of real-world programs. We provide scripts, Valgrind's stack traces as targets and initial seeds of each subject. Please follow the instructions to install fuzzers like [AFL(-QEMU)](https://lcamtuf.coredump.cx/afl/), [AFLGo](https://github.com/aflgo/aflgo) and [UAFuzz](https://github.com/uafuzz/UAFuzz).
~~~bash
# Environment variables
export AFL=/path/to/afl-2.52b
export AFLGO=/path/to/aflgo
export IDA_PATH=/path/to/ida-6.9/idaq
export GRAPH_EASY_PATH=/path/to/graph-easy
export UAFUZZ_PATH=/path/to/uafuzz

# Avoid hang when fuzzing
export MALLOC_CHECK_=0

# Checkout the benchmark
git clone https://github.com/strongcourage/uafbench.git
cd uafbench; export UAFBENCH_PATH=`pwd`

# Fuzz CVE-20018-20623 with UAFuzz and timeout 60 minutes
$UAFBENCH_PATH/CVE-2018-20623.sh uafuzz 60 $UAFBENCH_PATH/valgrind/CVE-2018-20623.valgrind

# Fuzz patched version of CVE-2018-6952
$UAFBENCH_PATH/CVE-2019-20633.sh uafuzz 360 $UAFBENCH_PATH/valgrind/CVE-2018-6952.valgrind
~~~

You can also fuzz without IDA Pro by specifying the argument `--no_ida` in the Python scripts. In this case, existing Ida files and call graphs in the folder [/ida](./ida) will be used. For example, the last two commands in [CVE-2019-20633.sh](./CVE-2019-20633.sh) should be updated as follows:
~~~bash
$UAFUZZ_PATH/scripts/preprocess.py --no_ida -f $PUT -v $targets -o $FUZZ_DIR
$UAFUZZ_PATH/scripts/run_uafuzz.py --no_ida -f $FUZZ_DIR/$PUT -M fuzz -i $FUZZ_DIR/in -o run -r "$FUZZ_DIR/$PUT -Rf" -I $runmode -T "$FUZZ_DIR/$PUT.tgt" -to $timeout
~~~

| Bug ID            | Program           |  Type   | Crash | Command | Files   |
|:-----------------:|:-----------------:|:-------:|:-----:|:-------:|:-------:|
| CVE-2018-20623 | readelf (923c6a7) | UAF | :x: | `readelf -a @@` | [PoC](./pocs/poc_CVE-2018-20623), [Traces](./valgrind/CVE-2018-20623.valgrind), [Fuzzing script](CVE-2018-20623.sh) |
| giflib-bug-74 | gifsponge (72e31ff) | DF | :x: | `gifsponge < @@` | [PoC](./pocs/poc_giflib-bug-74), [Traces](./valgrind/giflib-bug-74.valgrind), [Fuzzing script](giflib-bug-74.sh) |
| yasm-issue-91 | yasm (6caf151) | UAF | :x: | `yasm @@` | [PoC](./pocs/poc_yasm-issue-91), [Traces](./valgrind/yasm-issue-91.valgrind), [Fuzzing script](yasm-issue-91.sh) |
| CVE-2016-4487 | cxxfilt (2c49145) | UAF | :heavy_check_mark: | `cxxfilt < @@` | [PoC](./pocs/poc_CVE-2016-4487), [Traces](./valgrind/CVE-2016-4487.valgrind), [Fuzzing script](CVE-2016-4487.sh) |
| CVE-2018-11416 | jpegoptim (d23abf2) | DF | :x: | `jpegoptim @@` | [PoC](./pocs/poc_CVE-2018-11416), [Traces](./valgrind/CVE-2018-11416.valgrind), [Fuzzing script](CVE-2018-11416.sh) |
| mjs-issue-78 | mjs (9eae0e6) | UAF | :x: | `mjs -f @@` | [PoC](./pocs/poc_mjs-issue-78), [Traces](./valgrind/mjs-issue-78.valgrind), [Fuzzing script](mjs-issue-78.sh) |
| mjs-issue-73 | mjs (e4ea33a) | UAF | :x: | `mjs -f @@` | [PoC](./pocs/poc_mjs-issue-73), [Traces](./valgrind/mjs-issue-73.valgrind), [Fuzzing script](mjs-issue-73.sh) |
| CVE-2018-11496 | lzrip (ed51e14) | UAF | :x: | `lrzip -t @@` | [PoC](./pocs/poc_CVE-2018-11496), [Traces](./valgrind/CVE-2018-11496.valgrind), [Fuzzing script](CVE-2018-11496.sh) |
| CVE-2018-10685 | lzrip (9de7ccb) | UAF | :x: | `lrzip -t @@` | [PoC](./pocs/poc_CVE-2018-10685), [Traces](./valgrind/CVE-2018-10685.valgrind), [Fuzzing script](CVE-2018-10685.sh) |
| CVE-2019-6455 | rec2csv (97d20cc) | DF | :x: | `rec2csv @@` | [PoC](./pocs/poc_CVE-2019-6455), [Traces](./valgrind/CVE-2019-6455.valgrind), [Fuzzing script](CVE-2019-6455.sh) |
| CVE-2017-10686 | nasm (7a81ead) | UAF | :heavy_check_mark: | `nasm -f bin @@ -o /dev/null` | [PoC](./pocs/poc_CVE-2017-10686), [Traces](./valgrind/CVE-2017-10686.valgrind), [Fuzzing script](CVE-2017-10686.sh) |
| gifsicle-issue-122 | gifsicle (fad477c) | DF | :x: | `gifsicle @@ test.gif -o /dev/null` | [PoC](./pocs/poc_gifsicle-issue-122), [Traces](./valgrind/gifsicle-issue-122.valgrind), [Fuzzing script](gifsicle-issue-122.sh) |
| CVE-2016-3189 | bzip2 (962d606) | UAF | :heavy_check_mark: | `bzip2recover @@` | [PoC](./pocs/poc_CVE-2016-3189), [Traces](./valgrind/CVE-2016-3189.valgrind), [Fuzzing script](CVE-2016-3189.sh) |

## UAF bugs found by UAFuzz
| Bug ID            | Program           |  Type   | Command | Relevant bugs   |
|:-----------------:|:-----------------:|:-------:|:-------:|:---------------:|
| [CVE-2019-20633](https://savannah.gnu.org/bugs/index.php?56683) | patch | DF | `patch -Rf < @@` | [CVE-2018-6952](https://savannah.gnu.org/bugs/index.php?53133) |
| [#1269](https://github.com/gpac/gpac/issues/1269), [#1427](https://github.com/gpac/gpac/issues/1427), [#1440](https://github.com/gpac/gpac/issues/1440) | MP4Box | UAF | `MP4Box -info @@` | [#1340](https://github.com/gpac/gpac/issues/1340), [#1427](https://github.com/gpac/gpac/issues/1427) |
| [#702253](https://bugs.ghostscript.com/show_bug.cgi?id=702253) | mutool | UAF | `mutool draw -o /dev/null -R 832 -h 22 @@` | [#701294](https://bugs.ghostscript.com/show_bug.cgi?id=701294) |
| [#4266](https://github.com/fontforge/fontforge/issues/4266) | fontforge | UAF | `fontforge -lang=ff -c 'Open($1)' @@` | [#4084](https://github.com/fontforge/fontforge/issues/4084) |
| #134324, [#17117](https://github.com/Perl/perl5/issues/17117) | perl | UAF | `perl @@` | [#16889](https://github.com/Perl/perl5/issues/16889), [#17051](https://github.com/Perl/perl5/issues/17051) |
| [#25821](https://sourceware.org/bugzilla/show_bug.cgi?id=25821) | readelf | DF | `readelf -a @@` |  |
| [#25823](https://sourceware.org/bugzilla/show_bug.cgi?id=25823) | nm-new | UAF | `nm-new -C @@` |  |
|  | boolector | UAF | `boolector @@` | [#41](https://github.com/Boolector/boolector/issues/41) |






