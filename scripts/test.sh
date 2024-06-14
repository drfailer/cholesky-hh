#!/bin/bash

################################################################################
#                               global variables                               #
################################################################################

MATRIX_FILES_DIR=../../test-generator/scripts/cholesky0-10/
CHOLESKY=../build/cholesky-hh
RESULT_OUTPUT_DIR=./results

# if [ $# -eq 1 ]; then
#   RESULT_OUTPUT_DIR=./results-$1
# fi

BLOCK_SIZE_MIN=256
BLOCK_SIZE_MAX=256
NB_MEASURES=10
# threads for the tasks.
# strings: "d c u" (diagonal column update)
declare -a THREADS=()
# declare -a THREADS=(
#   "1 8 384"
#   "1 16 384"
#   "1 32 384"
#   "1 64 384"
# )

for ((i=5; i<=40; i+=5)); do
  THREADS+=("1 8 $i")
done

################################################################################
#                                running tests                                 #
################################################################################

mkdir -p $RESULT_OUTPUT_DIR
lscpu > $RESULT_OUTPUT_DIR/cpuinfo.txt

run_cholesky() {
  inputfile=$1
  graphname=$2                                                                                                                                                                                                       dThreads=$3
  cThreads=$4
  uThreads=$5
  blockSize=$6
  timefile=$7
  echo "$CHOLESKY -i $inputfile -g $graphname -d $dThreads -c $cThreads -u $uThreads -b $blockSize"
  echo "$CHOLESKY -i $inputfile -g $graphname -d $dThreads -c $cThreads -u $uThreads -b $blockSize" >> $timefile
  for ((i=0; i<$NB_MEASURES; i+=1)); do
    $CHOLESKY -i $inputfile -g $graphname -d $dThreads -c $cThreads -u $uThreads -b $blockSize >> $timefile
  done
}

# test the program for different matrix sizes
for threads in "${THREADS[@]}"; do
  dThreads=$(echo $threads | awk -F' ' '{ print $1 }')
  cThreads=$(echo $threads | awk -F' ' '{ print $2 }')
  uThreads=$(echo $threads | awk -F' ' '{ print $3 }')

  for ((blockSize=$BLOCK_SIZE_MIN; blockSize<=$BLOCK_SIZE_MAX; blockSize*=2)); do
    dirPath=$RESULT_OUTPUT_DIR/$dThreads-$cThreads-$uThreads/$blockSize
    mkdir -p $dirPath

    if [ $# -eq 1 ]; then
      graphname=$dirPath/$1.dot
      run_cholesky $MATRIX_FILES_DIR/$1.in $graphname $dThreads $cThreads $uThreads $blockSize $dirPath/times.txt
    else
      for file in $(ls $MATRIX_FILES_DIR); do
        graphname=$dirPath/$(echo $file | sed 's/\.in$/.dot/')
        run_cholesky $MATRIX_FILES_DIR/$file $graphname $dThreads $cThreads $uThreads $blockSize $dirPath/times.txt
      done
    fi
  done
done
