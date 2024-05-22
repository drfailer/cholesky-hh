#ifndef MATRIX_BLOCK_DATA_H
#define MATRIX_BLOCK_DATA_H
#include "block_types.h"
#include <cmath>
#include <cstddef>
#include <hedgehog/hedgehog.h>

template <typename T, BlockTypes BlockType> class MatrixBlockData {
public:
  MatrixBlockData(size_t blockSize, size_t x, size_t y, T *ptr, T *fullMatrix)
      : blockSize_(blockSize), x_(x), y_(y), ptr_(ptr),
        fullMatrix_(fullMatrix) {}

  size_t blockSize() const { return blockSize_; }

  size_t x() const { return x_; }
  size_t y() const { return y_; }
  void x(size_t x) { this->x_ = x; }
  void y(size_t y) { this->y_ = y; }

  T *fullMatrix() { return fullMatrix_; }
  T *get() { return ptr_; }

private:
  size_t blockSize_ = 0;
  size_t x_ = 0;
  size_t y_ = 0;
  T *ptr_ = nullptr;
  T *fullMatrix_ = nullptr;
};

#endif
