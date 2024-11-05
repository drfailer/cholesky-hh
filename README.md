# Cholesky Hedgeghog

## Requirements

- `gcc` 11.4
- `cmake`
- `OpenBlas`

## build

```sh
mkdir build
cd build
cmake ..
make
```

If your libraries are installed in a custom directory, you can use the option
`-DEXTERNAL_LIB_DIR=<PATH_TO_YOUR_LIB_DIR>` when running the cmake
configuration.

## Run

```sh
./cholesky-hh -i <INPUT_FILE> \
              -g <GRAPH_NAME> \
              -d <DIAGONAL_TASK_THREADS> \
              -c <COLUMN_TASK_THREADS> \
              -u <UPDATE_TASK_THREADS> \
              -b <BLOCK_SIZE>
```

## Measures

Measures can be done using `scripts/test.sh`. Using the script requires to
configure the following variables:

- `MATRIX_FILES_DIR`: path to the directory were the matrix files are stored
  (matrix files are generated using [test-generator](https://github.com/drfailer/test-generator)).
- `CHOLESKY`: path to cholesky-hh executable.
- `RESULT_OUTPUT_DIR`: base name of the output directory.
- `NB_MEASURES`: number of measures for a configuration

### Manual testing

The function `manual_test` in the test script can be used to try very specific
configurations. By default, `automatic_test` is used because it is faster (the
configurations are hardcoded in the program). However, it is still possible to
use manual testing, but this requires to re-execute the program for each
configuration (re-executing means re-loading the matrix file).

The following global variables can be used to create configurations:

- the block size start from `BLOCK_SIZE_MIN` and is multiplied by 2 at each
  iteration till `BLOCK_SIZE_MAX` is reached.
- `THREADS`: array of strings in which each string contains the number of
  threads for the different tasks separated by spaces (see `cholesky-hh -h` for
  more information on the configuration).

## Parsing the results

The `parse-result` script requires the results from lapack obtained using
[this code](https://github.com/drfailer/cholesky).

Usage of the script:

```sh
parse-result <HHOutputDir> <Size> <blockSize> <LapackFile>
```
