#!/usr/bin/env bash

################################################################################
#                               global variables                               #
################################################################################

MATRIX_FILES_DIR=../../test-generator/scripts/cholesky/
CHOLESKY=../build/cholesky-hh
RESULT_OUTPUT_DIR=./results

BLOCK_SIZE_MIN=128
BLOCK_SIZE_MAX=1024

# threads for the tasks.
# strings: "d c u" (diagonal column update)
declare -a THREADS=(
  "1 8 32" "1 16 24" "1 20 20"
)

################################################################################
#                                running tests                                 #
################################################################################

mkdir -p $RESULT_OUTPUT_DIR
lscpu > $RESULT_OUTPUT_DIR/cpuinfo.txt

# test the program for different matrix sizes
for threads in "${THREADS[@]}"; do
  dThreads=$(echo $threads | awk -F' ' '{ print $1 }')
  cThreads=$(echo $threads | awk -F' ' '{ print $2 }')
  uThreads=$(echo $threads | awk -F' ' '{ print $3 }')

  for ((blockSize=$BLOCK_SIZE_MIN; blockSize<=$BLOCK_SIZE_MAX; blockSize*=2)); do
    dirPath=$RESULT_OUTPUT_DIR/$dThreads-$cThreads-$uThreads/$blockSize
    mkdir -p $dirPath

    for file in $(ls $MATRIX_FILES_DIR); do
      graphname=$dirPath/$(echo $file | sed 's/\.in$/.dot/')

      echo "$CHOLESKY -i $MATRIX_FILES_DIR/$file -g $graphname -d $dThreads -c $cThreads -u $uThreads -b $blockSize" >> $dirPath/times.txt
      for i in {1..10}; do
        $CHOLESKY -i $MATRIX_FILES_DIR/$file -g $graphname -d $dThreads -c $cThreads -u $uThreads -b $blockSize >> $dirPath/times.txt
      done

    done
  done
done
