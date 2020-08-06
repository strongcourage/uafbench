#!/bin/bash
PUT="giflib-bug-74"
runmode=$1
timeout=$2
targets=$3

# Checkout source code
git clone https://git.code.sf.net/p/giflib/code $PUT
cd $PUT; export SUBJECT=$PWD;
git checkout 72e31ff

# Compile source code
./autogen.sh; make distclean
rm -rf obj; mkdir obj; cd obj
CFLAGS="-g -m32" CXXFLAGS="-g -m32" ../configure --disable-shared --prefix=`pwd`
make clean; make -j4
cd $SUBJECT; rm -rf obj-asan; mkdir obj-asan; cd obj-asan
CFLAGS="-g -m32 -fsanitize=address" CXXFLAGS="-g -m32 -fsanitize=address" ../configure --disable-shared --prefix=`pwd`
make clean; make

# Prepare working directories
cd $SUBJECT; rm -rf obj-$runmode; mkdir obj-$runmode; export FUZZ_DIR=$SUBJECT/obj-$runmode; cd $FUZZ_DIR
mkdir in; cp $UAFBENCH_PATH/seeds/$PUT/* in

# Fuzzing
if [ $runmode = "aflqemu" ]; then
	cp $SUBJECT/obj/util/gifsponge .
	timeout -sHUP ${timeout}m $SCRIPTS/run_afl.py -f $FUZZ_DIR/gifsponge -Q -i $FUZZ_DIR/in -o run -r "$FUZZ_DIR/gifsponge" -to $timeout
elif [ $runmode = "aflgo" ]; then
	SECONDS=0
	export AFL_PATH=$HOME/aflgo; export AFLGO=$AFL_PATH
	mkdir temp; export TMP_DIR=$FUZZ_DIR/temp
	export CC=$AFLGO/afl-clang-fast; export CXX=$AFLGO/afl-clang-fast++
	export LDFLAGS=-lpthread
	export ADDITIONAL="-targets=$TMP_DIR/BBtargets.txt -outdir=$TMP_DIR -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"
	echo $'gifsponge.c:44\negif_lib.c:92\ngifsponge.c:72\negif_lib.c:1144\negif_lib.c:802\ngifsponge.c:77\negif_lib.c:764' > $TMP_DIR/BBtargets.txt
	CFLAGS="$ADDITIONAL" CXXFLAGS="$ADDITIONAL" ../configure --disable-shared --prefix=`pwd`
	make clean; make
	cat $TMP_DIR/BBnames.txt | rev | cut -d: -f2- | rev | sort | uniq > $TMP_DIR/BBnames2.txt && mv $TMP_DIR/BBnames2.txt $TMP_DIR/BBnames.txt
	cat $TMP_DIR/BBcalls.txt | sort | uniq > $TMP_DIR/BBcalls2.txt && mv $TMP_DIR/BBcalls2.txt $TMP_DIR/BBcalls.txt
	$AFLGO/scripts/genDistance.sh $SUBJECT $TMP_DIR gifsponge
	CFLAGS="-distance=$TMP_DIR/distance.cfg.txt" CXXFLAGS="-distance=$TMP_DIR/distance.cfg.txt" ../configure --disable-shared --prefix=`pwd`
	make clean; make
	pp_aflgo_time=$SECONDS; echo "pp_aflgo_time: $pp_aflgo_time (s)."
	timeout -sHUP "${timeout}m" $AFLGO/afl-fuzz -m none -z exp -c 45m -i in -o out $FUZZ_DIR/util/gifsponge
elif [ $1 = "aflgob" ] || [ $1 = "heb" ] || [ $1 = "uafuzz" ]; then
	cp $SUBJECT/obj/util/gifsponge $PUT; cp $targets .
	$UAFUZZ_PATH/scripts/preprocess.py -f $PUT -v $targets -o $FUZZ_DIR
	$UAFUZZ_PATH/scripts/run_uafuzz.py -f $FUZZ_DIR/$PUT -M fuzz -i $FUZZ_DIR/in -o run -r "$FUZZ_DIR/$PUT" -I $runmode -T "$FUZZ_DIR/$PUT.tgt" -to $timeout
fi
