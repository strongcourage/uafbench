#!/bin/bash
PUT="gifsicle-issue-122"
runmode=$1
timeout=$2
targets=$3

# Checkout source code
git clone https://github.com/kohler/gifsicle.git $PUT
cd $PUT; export SUBJECT=$PWD;
git checkout fad477c

# Compile source code
./bootstrap.sh; make distclean
rm -rf obj; mkdir obj; cd obj
CFLAGS="-g -m32 -pthread" ../configure --prefix=`pwd`
make clean; make
cd $SUBJECT; rm -rf obj-asan; mkdir obj-asan; cd obj-asan
CFLAGS="-g -m32 -fsanitize=address -pthread" ../configure --prefix=`pwd`
make clean; make

# Prepare working directories
cd $SUBJECT; rm -rf obj-$runmode; mkdir obj-$runmode; export FUZZ_DIR=$SUBJECT/obj-$runmode; cd $FUZZ_DIR
mkdir in; cp $UAFBENCH_PATH/seeds/$PUT/* in

# Fuzzing
if [ $runmode = "aflqemu" ]; then
	cp $SUBJECT/obj/src/gifsicle .
	timeout -sHUP ${timeout}m $SCRIPTS/run_afl.py -f $FUZZ_DIR/gifsicle -Q -i $FUZZ_DIR/in -o run -r "$FUZZ_DIR/gifsicle @@ $UAFBENCH_PATH/pocs/test.gif -o /dev/null" -to $timeout
elif [ $runmode = "aflgo" ]; then
	SECONDS=0
	export AFL_PATH=$HOME/aflgo; export AFLGO=$AFL_PATH
	mkdir temp; export TMP_DIR=$FUZZ_DIR/temp
	export CC=$AFLGO/afl-clang-fast; export CXX=$AFLGO/afl-clang-fast++
	export LDFLAGS=-lpthread
	export ADDITIONAL="-targets=$TMP_DIR/BBtargets.txt -outdir=$TMP_DIR -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"
	echo $'gifsicle.c:2144\ngifsicle.c:735\ngifread.c:929\ngifread.c:853\ngifread.c:671\nfmalloc.c:19\ngifread.c:898\ngifsicle.c:2180\nsupport.c:1741\ngiffunc.c:502\ngiffunc.c:529' > $TMP_DIR/BBtargets.txt
	CFLAGS="$ADDITIONAL" CXXFLAGS="$ADDITIONAL" ../configure --disable-shared --prefix=`pwd`
	make clean; make;
	cat $TMP_DIR/BBnames.txt | rev | cut -d: -f2- | rev | sort | uniq > $TMP_DIR/BBnames2.txt && mv $TMP_DIR/BBnames2.txt $TMP_DIR/BBnames.txt
	cat $TMP_DIR/BBcalls.txt | sort | uniq > $TMP_DIR/BBcalls2.txt && mv $TMP_DIR/BBcalls2.txt $TMP_DIR/BBcalls.txt
	$AFLGO/scripts/genDistance.sh $SUBJECT $TMP_DIR gifsicle
	CFLAGS="-distance=$TMP_DIR/distance.cfg.txt" CXXFLAGS="-distance=$TMP_DIR/distance.cfg.txt" ../configure --disable-shared --prefix=`pwd`
	make clean; make;
	pp_aflgo_time=$SECONDS; echo "pp_aflgo_time: $pp_aflgo_time (s)."
	timeout -sHUP "${timeout}m" $AFLGO/afl-fuzz -m none -z exp -c 45m -i $FUZZ_DIR/in -o out $FUZZ_DIR/src/gifsicle @@ $UAFBENCH_PATH/pocs/test.gif -o /dev/null
elif [ $1 = "aflgob" ] || [ $1 = "heb" ] || [ $1 = "uafuzz" ]; then
	cp $SUBJECT/obj/src/gifsicle $PUT; cp $targets .
	$UAFUZZ_PATH/scripts/preprocess.py -f $PUT -v $targets -o $FUZZ_DIR
	$UAFUZZ_PATH/scripts/run_uafuzz.py -f $FUZZ_DIR/$PUT -M fuzz -i $FUZZ_DIR/in -o run -r "$FUZZ_DIR/$PUT @@ $UAFBENCH_PATH/pocs/test.gif -o /dev/null" -I $runmode -T "$FUZZ_DIR/$PUT.tgt" -to $timeout
fi
