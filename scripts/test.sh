#!/usr/bin/env bash                                                                                                                                                                                       [10/1923]
################################################################################
#                               global variables                               #
################################################################################

MATRIX_FILES_DIR=/scratch/rvc1/samples
CHOLESKY=/scratch/rvc1/cholesky-hh
RESULT_OUTPUT_DIR=/scratch/rvc1/results-hh

if [ $# -eq 1 ]; then
  RESULT_OUTPUT_DIR=/scratch/rvc1/results-hh-$1
fi

BLOCK_SIZE_MIN=256
BLOCK_SIZE_MAX=256
NB_MEASURES=5
# threads for the tasks.
# strings: "d c u" (diagonal column update)
declare -a THREADS=(
  "1 8 10 2 20"
  "1 8 15 2 20"
  "1 8 20 2 20"
  "1 8 25 2 20"
  "1 8 30 2 20"
  "1 8 35 2 20"
  "1 8 40 2 20"
)

################################################################################
#                                running tests                                 #
################################################################################

mkdir -p $RESULT_OUTPUT_DIR
lscpu > $RESULT_OUTPUT_DIR/cpuinfo.txt

run_cholesky() {
  inputfile=$1
  graphname=$2                                                                                                                                                                                                       dThreads=$3
  dThreads=$1
  cThreads=$4
  uThreads=$5
  sThreads=$6
  vThreads=$7
  blockSize=$8
  timefile=$9
  echo "$CHOLESKY -i $inputfile -g $graphname -d $dThreads -c $cThreads -u $uThreads -s $sThreads -v $vThreads -b $blockSize" >> $timefile
  for ((i=0; i<$NB_MEASURES; i+=1)); do
    $CHOLESKY -i $inputfile -g $graphname -d $dThreads -c $cThreads -u $uThreads -s $sThreads -v $vThreads -b $blockSize >> $timefile
  done
}

# test the program for different matrix sizes
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
