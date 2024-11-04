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
