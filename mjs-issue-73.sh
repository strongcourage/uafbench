#!/bin/bash
PUT="mjs-issue-73"
runmode=$1
timeout=$2
targets=$3

# Checkout source code
git clone https://github.com/cesanta/mjs.git $PUT
cd $PUT; export SUBJECT=$PWD;
git checkout e4ea33a

# Compile source code
gcc -DMJS_MAIN -m32 mjs.c -ldl -g -o mjs-bin
gcc -DMJS_MAIN -m32 -fsanitize=address mjs.c -ldl -g -o mjs-asan

# Prepare working directories
cd $SUBJECT; rm -rf obj-$runmode; mkdir obj-$runmode; export FUZZ_DIR=$SUBJECT/obj-$runmode; cd $FUZZ_DIR
mkdir in; cp $UAFBENCH_PATH/seeds/$PUT/* in

# Fuzzing
if [ $runmode = "aflqemu" ]; then
	cp $SUBJECT/mjs-bin .
	timeout -sHUP ${timeout}m $SCRIPTS/run_afl.py -f $FUZZ_DIR/mjs-bin -Q -i $FUZZ_DIR/in -o run -r "$FUZZ_DIR/mjs-bin -f @@" -to $timeout
elif [ $runmode = "aflgo" ]; then
	SECONDS=0
	export AFL_PATH=$HOME/aflgo; export AFLGO=$AFL_PATH
	mkdir temp; export TMP_DIR=$FUZZ_DIR/temp
	export CC=$AFLGO/afl-clang-fast; export CXX=$AFLGO/afl-clang-fast++
	export LDFLAGS=-lpthread
	export ADDITIONAL="-targets=$TMP_DIR/BBtargets.txt -outdir=$TMP_DIR -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"
	echo $'mjs.c:12068\nmjs.c:9781\nmjs.c:9758\nmjs.c:9418\nmjs.c:13706\nmjs.c:4924\nmjs.c:9541\nmjs.c:12012\nmjs.c:11963\nmjs.c:6434\nmjs.c:6058\nmjs.c:5976\nmjs.c:5936\nmjs.c:5970\nmjs.c:5884\nmjs.c:11870\nmjs.c:5938\nmjs.c:5794\nmjs.c:5790' > $TMP_DIR/BBtargets.txt
	cd $SUBJECT; $CC -DMJS_MAIN mjs.c $ADDITIONAL -ldl -g -o mjs-bin
	cat $TMP_DIR/BBnames.txt | rev | cut -d: -f2- | rev | sort | uniq > $TMP_DIR/BBnames2.txt && mv $TMP_DIR/BBnames2.txt $TMP_DIR/BBnames.txt
	cat $TMP_DIR/BBcalls.txt | sort | uniq > $TMP_DIR/BBcalls2.txt && mv $TMP_DIR/BBcalls2.txt $TMP_DIR/BBcalls.txt
	$AFLGO/scripts/genDistance.sh $SUBJECT $TMP_DIR mjs-bin
	$CC -DMJS_MAIN mjs.c -distance=$TMP_DIR/distance.cfg.txt -ldl -g -o mjs-bin
	cd $FUZZ_DIR; cp $SUBJECT/mjs-bin .
	pp_aflgo_time=$SECONDS; echo "pp_aflgo_time: $pp_aflgo_time (s)."
	timeout -sHUP "${timeout}m" $AFLGO/afl-fuzz -m none -z exp -c 45m -i in -o out $FUZZ_DIR/mjs-bin -f @@
elif [ $1 = "aflgob" ] || [ $1 = "heb" ] || [ $1 = "uafuzz" ]; then
	cp $SUBJECT/mjs-bin $PUT; cp $targets .
	$UAFUZZ_PATH/scripts/preprocess.py -f $PUT -v $targets -o $FUZZ_DIR
	$UAFUZZ_PATH/scripts/run_uafuzz.py -f $FUZZ_DIR/$PUT -M fuzz -i $FUZZ_DIR/in -o run -r "$FUZZ_DIR/$PUT -f @@" -I $runmode -T "$FUZZ_DIR/$PUT.tgt" -to $timeout
fi
