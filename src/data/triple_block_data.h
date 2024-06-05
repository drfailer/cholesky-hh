#ifndef CHOLESKY_HH_TRIPLE_BLOCK_DATA_H
#define CHOLESKY_HH_TRIPLE_BLOCK_DATA_H

#include "./matrix_block_data.h"

/// @brief Container that is used to send data to the update task.
template<typename T>
struct TripleBlockData {
  TripleBlockData(std::shared_ptr<MatrixBlockData<T, Block>> first,
                  std::shared_ptr<MatrixBlockData<T, Block>> second,
                  std::shared_ptr<MatrixBlockData<T, Block>> third) :
          first(first), second(second), third(third) {}

  std::shared_ptr<MatrixBlockData<T, Block>> first = nullptr;
  std::shared_ptr<MatrixBlockData<T, Block>> second = nullptr;
  std::shared_ptr<MatrixBlockData<T, Block>> third = nullptr;
};

#endif //CHOLESKY_HH_TRIPLE_BLOCK_DATA_H
