#!/usr/bin/env bash

# NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
# software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
# derivative works of the software or any portion of the software, and you may copy and distribute such modifications
# or works. Modified works should carry a notice stating that you changed the software and should note the date and
# nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
# source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
# EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
# WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
# CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
# THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
# are solely responsible for determining the appropriateness of using and distributing the software and you assume
# all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
# with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of
# operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
# damage to property. The software developed by NIST employees is not subject to copyright protection within the
# United States.

# avoid openblas crashing when over-subscribing threads.
export OPENBLAS_NUM_THREADS=1

################################################################################
#                               global variables                               #
################################################################################

MATRIX_FILES_DIR=../../test-generator/scripts/cholesky/
CHOLESKY=../build/cholesky-hh
RESULT_OUTPUT_DIR=./results-hh

if [ $# -eq 1 ]; then
  RESULT_OUTPUT_DIR=./results-hh-$1
fi

BLOCK_SIZE_MIN=256
BLOCK_SIZE_MAX=256
NB_MEASURES=10
# threads for the tasks.
# strings: "d c u s v" (diagonal column update)
declare -a THREADS=(
  "1 8 10 1 20"
  "1 8 15 1 20"
  "1 8 20 1 20"
  "1 8 25 1 20"
  "1 8 30 1 20"
  "1 8 35 1 20"
  "1 8 40 1 20"
)

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
  sThreads=$6
  vThreads=$7
  blockSize=$8
  timefile=$9
  echo "$CHOLESKY -i $inputfile -g $graphname -d $dThreads -c $cThreads -u $uThreads -s $sThreads -v $vThreads -b $blockSize"
  echo "$CHOLESKY -i $inputfile -g $graphname -d $dThreads -c $cThreads -u $uThreads -s $sThreads -v $vThreads -b $blockSize" >> $timefile
  for ((i=0; i<$NB_MEASURES; i+=1)); do
    $CHOLESKY -i $inputfile -g $graphname -d $dThreads -c $cThreads -u $uThreads -s $sThreads -v $vThreads -b $blockSize >> $timefile
  done
}

# Tests the program for different matrix sizes and threads configurations.
# Requires to reload the matrix file for each measure.
manual_test() {
  for threads in "${THREADS[@]}"; do
    dThreads=$(echo $threads | awk -F' ' '{ print $1 }')
    cThreads=$(echo $threads | awk -F' ' '{ print $2 }')
    uThreads=$(echo $threads | awk -F' ' '{ print $3 }')
    sThreads=$(echo $threads | awk -F' ' '{ print $4 }')
    vThreads=$(echo $threads | awk -F' ' '{ print $5 }')

    for ((blockSize=$BLOCK_SIZE_MIN; blockSize<=$BLOCK_SIZE_MAX; blockSize*=2)); do
      dirPath=$RESULT_OUTPUT_DIR/$dThreads-$cThreads-$uThreads-$sThreads-$vThreads/$blockSize
      mkdir -p $dirPath

      if [ $# -eq 1 ]; then
        graphname=$dirPath/$1.dot
        run_cholesky $MATRIX_FILES_DIR/$1.in $graphname $dThreads $cThreads $uThreads $sThreads $vThreads $blockSize $dirPath/times.txt
      else
        for file in $(ls $MATRIX_FILES_DIR); do
          graphname=$dirPath/$(echo $file | sed 's/\.in$/.dot/')
          run_cholesky $MATRIX_FILES_DIR/$file $graphname $dThreads $cThreads $uThreads $sThreads $vThreads $blockSize $dirPath/times.txt
        done
      fi
    done
  done
}

# Run the test compiled inside the program (avoid reloading the matrix file).
# Note: the output form is different compared to the manual version.
automatic_test() {
  for ((blockSize=$BLOCK_SIZE_MIN; blockSize<=$BLOCK_SIZE_MAX; blockSize*=2)); do
    timefile=$RESULT_OUTPUT_DIR/times-$1-$blockSize.txt
    echo "$CHOLESKY -g $RESULT_OUTPUT_DIR -i $MATRIX_FILES_DIR/$1.in -l 1 -b $blockSize >> $timefile"
    $CHOLESKY -g $RESULT_OUTPUT_DIR -i $MATRIX_FILES_DIR/$1.in -l 1 -b $blockSize >> $timefile
  done
}

if [ $# -eq 1 ]; then
  # manual_test $1
  automatic_test $1
else
  echo "error: requires size as argument."
fi
