#!/bin/bash

# TODO: Put this in a test prep script?

# Ensure the data directory has 750 permissions
chmod 750 data

# Must run with env -i so that students cannot "accidentally" use the
# standard shell environment variables
# HACK HACK HACK for MAC MAC MAC
# Had to add /bin to the PATH...remove on Linux and update expected files
EXE="env -i PATH=/usr/bin:/bin:. ../dukesh"

function run_test {

    # parameters
    TAG=$1
		LINE=$(awk '$3 == "'${TAG}'"' itests.include | grep -v ^#)
		if [[ $LINE == "" ]] ; then
		    echo "ERROR: tag ${TAG} not found in itests.include"
				return 127
		fi
		if [ $(echo $LINE | wc -l) != 1 ] ; then
		    echo "ERROR: tag ${TAG} does not have a unique entry in itests.include"
				return 127
		fi
    ARGS=$(echo $LINE | cut -d'"' -f2)

    PTAG=$(printf '%-30s' "$TAG")

    # file paths
    OUTPUT=outputs/$TAG.txt
    FAIL=outputs/.$TAG.fail
    EXPECT=expected/$TAG.txt
    VALGRND=valgrind/$TAG.txt

    # run test and compare output to the expected version
    $EXE $ARGS 2>/dev/null >"$OUTPUT"

    EFILES="$EXPECT $(find expected -type f -name "$TAG-*.txt")"
    for EXP in $EFILES ; do
			  DIFF=$(echo $EXP | sed 's/expected/outputs/' | sed 's/txt/diff/')
        diff -u "$OUTPUT" "$EXP" >"$DIFF"
        if [ ! -s "$DIFF" ]; then
            echo "$PTAG pass"
            rm outputs/$TAG*diff
            if [ -a $FAIL ] ; then
                rm $FAIL
            fi

            # run valgrind
            #valgrind $EXE $ARGS &>$VALGRND

            return 0
        fi
    done

    echo "$PTAG FAIL"
		printf '%4s' " "
		echo "Command line: $EXE $ARGS" | tee $FAIL
    return 1

}

# structure of itests.include:
# LEVEL FEATURE TEST "args"

if [[ $1 = "info" ]] ; then
    #     if LEVEL, "Grade level $LEVEL has $n features: $f1 ...
    #     if FEATURE, "Feature $FEATURE has $n tests: $t1 ...
    #     if TEST, "Test $TEST has arguments \"...\""
    echo "list all test cases"
		exit 0
fi

# initialize output folders
if [ ! -d "outputs" ] ; then
    mkdir -p outputs
fi
#if [ ! -d "valgrind" ] ; then
#  mkdir -p valgrind
#fi
#rm -f outputs/* valgrind/*

TESTS=$@

echo "----------------------------------------"
for t in $TESTS ; do
    run_test $t
    if [[ $? != 0 ]] ; then
        echo "----------------------------------------"
        #echo "ERROR: Stopping due to failing test $t"
        exit 1
    fi
done

echo "----------------------------------------"
echo "SUCCESS: All tests passed"
exit 0

# run individual tests
#source itests.include

# check for memory leaks
#LEAK=`cat valgrind/*.txt | grep 'definitely lost' | grep -v ' 0 bytes in 0 blocks'`
#if [ -z "$LEAK" ]; then
#    echo "No memory leak found."
#else
#    echo "Memory leak(s) found. See files listed below for details."
#    grep 'definitely lost' valgrind/*.txt | grep -v ' 0 bytes in 0 blocks' | sed -e 's/:.*$//g' | sed -e 's/^/  - /g'
#fi

