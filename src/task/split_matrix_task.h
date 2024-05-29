#ifndef SPLIT_MATRIX_TASK_H
#define SPLIT_MATRIX_TASK_H

#include "../data/matrix_block_data.h"
#include "../data/matrix_data.h"
#include <hedgehog/hedgehog.h>

#define SMTaskInNb 1
#define SMTaskIn MatrixData<T>
#define SMTaskOut MatrixBlockData<T, BlockType>

template<typename T, BlockTypes BlockType>
class SplitMatrixTask
        : public hh::AbstractTask<SMTaskInNb, SMTaskIn, SMTaskOut > {
 public:
  SplitMatrixTask()
          : hh::AbstractTask<SMTaskInNb, SMTaskIn, SMTaskOut >("Split matrix task") {
  }

  void execute(std::shared_ptr<MatrixData<T>> matrix) override {
    for (size_t iBlock = 0; iBlock < matrix->nbBlocksRows(); ++iBlock) {
      for (size_t jBlock = 0; jBlock <= iBlock; ++jBlock) {
        this->addResult(std::make_shared<MatrixBlockData<T, BlockType>>(
                matrix->blockSize(), matrix->nbBlocksRows(), matrix->nbBlocksCols(),
                jBlock, iBlock, matrix->width(), matrix->height(),
                matrix->get() + iBlock * matrix->blockSize() * matrix->width() +
                jBlock * matrix->blockSize(), matrix->get()));
      }
    }
  }
};

#endif
