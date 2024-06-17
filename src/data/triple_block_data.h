#ifndef CHOLESKY_HH_TRIPLE_BLOCK_DATA_H
#define CHOLESKY_HH_TRIPLE_BLOCK_DATA_H

#include "./matrix_block_data.h"

/// @brief Container that is used to send data to the update task.
template <typename T, BlockTypes B1 = MatrixBlock, BlockTypes B2 = MatrixBlock, BlockTypes B3 = MatrixBlock>
struct TripleBlockData {
  TripleBlockData(std::shared_ptr<MatrixBlockData<T, B1>> first,
                  std::shared_ptr<MatrixBlockData<T, B2>> second,
                  std::shared_ptr<MatrixBlockData<T, B3>> third) :
          first(first), second(second), third(third) {}

  std::shared_ptr<MatrixBlockData<T, B1>> first = nullptr;
  std::shared_ptr<MatrixBlockData<T, B2>> second = nullptr;
  std::shared_ptr<MatrixBlockData<T, B3>> third = nullptr;
};

#endif //CHOLESKY_HH_TRIPLE_BLOCK_DATA_H
