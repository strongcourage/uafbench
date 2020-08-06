#!/bin/bash
PUT="yasm-issue-91"
runmode=$1
timeout=$2
targets=$3

# Checkout source code
git clone https://github.com/yasm/yasm.git $PUT
cd $PUT; export SUBJECT=$PWD;
git checkout 6caf151
	
# Compile source code
./autogen.sh; make distclean
rm -rf obj; mkdir obj; cd obj
CFLAGS="-g -m32" CXXFLAGS="-g -m32" ../configure --disable-shared --prefix=`pwd`
make clean; make
cd $SUBJECT; rm -rf obj-asan; mkdir obj-asan; cd obj-asan
CFLAGS="-g -m32 -fsanitize=address" CXXFLAGS="-g -m32 -fsanitize=address" ../configure --disable-shared --prefix=`pwd`
make clean; make

# Prepare working directories
cd $SUBJECT; rm -rf obj-$runmode; mkdir obj-$runmode; export FUZZ_DIR=$SUBJECT/obj-$runmode; cd $FUZZ_DIR
mkdir in; cp $UAFBENCH_PATH/seeds/$PUT/* in

# Fuzzing
if [ $runmode = "aflqemu" ]; then
	cp $SUBJECT/obj/yasm .
	timeout -sHUP ${timeout}m $SCRIPTS/run_afl.py -f $FUZZ_DIR/yasm -Q -i $FUZZ_DIR/in -o run -r "$FUZZ_DIR/yasm @@" -to $timeout
elif [ $runmode = "aflgo" ]; then
	SECONDS=0
	export AFL_PATH=$HOME/aflgo; export AFLGO=$AFL_PATH
	mkdir temp; export TMP_DIR=$FUZZ_DIR/temp
	export CC=$AFLGO/afl-clang-fast; export CXX=$AFLGO/afl-clang-fast++
	export LDFLAGS=-lpthread
	export ADDITIONAL="-targets=$TMP_DIR/BBtargets.txt -outdir=$TMP_DIR -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"
	echo $'intnum.c:415\nexpr.c:1131\nexpr.c:582\nxmalloc.c:113\nintnum.c:417\nexpr.c:689\nexpr.c:986\nexpr.c:1020\nxmalloc.c:69\nintnum.c:397\nexpr.c:1089\nexpr.c:1120\nexpr.c:834\nexpr.c:1019\ninsn.c:256\nx86id.c:1082\nbytecode.c:176\nsection.c:535\nyasm.c:527\nyasm.c:752' > $TMP_DIR/BBtargets.txt
	CFLAGS="$ADDITIONAL" CXXFLAGS="$ADDITIONAL" ../configure --disable-shared --prefix=`pwd`
	make clean; make
	cat $TMP_DIR/BBnames.txt | rev | cut -d: -f2- | rev | sort | uniq > $TMP_DIR/BBnames2.txt && mv $TMP_DIR/BBnames2.txt $TMP_DIR/BBnames.txt
	cat $TMP_DIR/BBcalls.txt | sort | uniq > $TMP_DIR/BBcalls2.txt && mv $TMP_DIR/BBcalls2.txt $TMP_DIR/BBcalls.txt
	$AFLGO/scripts/genDistance.sh $SUBJECT $TMP_DIR yasm
	CFLAGS="-distance=$TMP_DIR/distance.cfg.txt" CXXFLAGS="-distance=$TMP_DIR/distance.cfg.txt" ../configure --disable-shared --prefix=`pwd`
	make clean; make
	pp_aflgo_time=$SECONDS; echo "pp_aflgo_time: $pp_aflgo_time (s)."
	timeout -sHUP "${timeout}m" $AFLGO/afl-fuzz -m none -z exp -c 45m -i $FUZZ_DIR/in -o out $FUZZ_DIR/yasm @@
elif [ $1 = "aflgob" ] || [ $1 = "heb" ] || [ $1 = "uafuzz" ]; then
	cp $SUBJECT/obj/yasm $PUT; cp $targets .
	$UAFUZZ_PATH/scripts/preprocess.py -f $PUT -v $targets -o $FUZZ_DIR
	$UAFUZZ_PATH/scripts/run_uafuzz.py -f $FUZZ_DIR/$PUT -M fuzz -i $FUZZ_DIR/in -o run -r "$FUZZ_DIR/$PUT @@" -I $runmode -T "$FUZZ_DIR/$PUT.tgt" -to $timeout
fi
