#ifndef CHOLESKY_HH_TRIPLE_BLOCK_DATA_H
#define CHOLESKY_HH_TRIPLE_BLOCK_DATA_H

#include "./matrix_block_data.h"

template<typename T>
struct TripleBlockData {
  TripleBlockData(std::shared_ptr<MatrixBlockData<T, Result>> first,
                  std::shared_ptr<MatrixBlockData<T, Result>> second,
                  std::shared_ptr<MatrixBlockData<T, Block>> third) :
          first(first), second(second), third(third) {}

  std::shared_ptr<MatrixBlockData<T, Result>> first = nullptr;
  std::shared_ptr<MatrixBlockData<T, Result>> second = nullptr;
  std::shared_ptr<MatrixBlockData<T, Block>> third = nullptr;
};

#endif //CHOLESKY_HH_TRIPLE_BLOCK_DATA_H
