#ifndef SPLIT_MATRIX_TASK_H
#define SPLIT_MATRIX_TASK_H

#include <hedgehog/hedgehog.h>
#include <memory>
#include "../data/matrix_block_data.h"
#include "../data/matrix_data.h"

#define SMTaskInNb 2
#define SMTaskIn MatrixData<T, MatrixTypes::Matrix>, MatrixData<T, MatrixTypes::Vector>
#define SMTaskOut MatrixBlockData<T, BlockType>

template <typename T, BlockTypes BlockType>
class SplitMatrixTask
        : public hh::AbstractAtomicTask<SMTaskInNb, SMTaskIn, SMTaskOut > {
 public:
  SplitMatrixTask()
          : hh::AbstractAtomicTask<SMTaskInNb, SMTaskIn, SMTaskOut >("Split matrix task") {
  }

  void execute(std::shared_ptr<MatrixData<T, MatrixTypes::Matrix>> matrix) override {
    for (size_t iBlock = 0; iBlock < matrix->nbBlocksRows(); ++iBlock) {
      for (size_t jBlock = 0; jBlock <= iBlock; ++jBlock) {
        this->addResult(std::make_shared<MatrixBlockData<T, BlockType>>(
                std::min(matrix->blockSize(), matrix->width() - (jBlock * matrix->blockSize())),
                std::min(matrix->blockSize(), matrix->height() - (iBlock * matrix->blockSize())),
                matrix->nbBlocksRows(), matrix->nbBlocksCols(),
                jBlock, iBlock, matrix->width(), matrix->height(),
                matrix->get() + iBlock * matrix->blockSize() * matrix->width() +
                jBlock * matrix->blockSize(), matrix->get()));
      }
    }
  }

  void execute(std::shared_ptr<MatrixData<T, MatrixTypes::Vector>> vector) override {
    for (size_t iBlock = 0; iBlock < vector->nbBlocksRows(); ++iBlock) {
      this->addResult(std::make_shared<MatrixBlockData<T, BlockType>>(
              std::min(vector->blockSize(), vector->width()),
              std::min(vector->blockSize(), vector->height() - (iBlock * vector->blockSize())),
              vector->nbBlocksRows(), vector->nbBlocksCols(), 0, iBlock, vector->width(),
              vector->height(), vector->get() + iBlock * vector->blockSize() * vector->width(),
              vector->get()));
    }
  }
};

#endif
