#ifndef MATRIX_BLOCK_DATA_H
#define MATRIX_BLOCK_DATA_H
#include "block_types.h"
#include <cmath>
#include <cstddef>
#include <hedgehog/hedgehog.h>

template <typename T, BlockTypes BlockType> class MatrixBlockData {
public:
  MatrixBlockData(size_t blockSize, size_t nbBlocksRows, size_t nbBlocksCols,
                  size_t x, size_t y, size_t matrixWidth, size_t matrixHeight,
                  T *ptr, T *fullMatrix)
      : blockSize_(blockSize), nbBlocksRows_(nbBlocksRows),
        nbBlocksCols_(nbBlocksCols), x_(x), y_(y), matrixWidth_(matrixWidth),
        matrixHeight_(matrixHeight), ptr_(ptr), fullMatrix_(fullMatrix) {}

  template <typename Other, BlockTypes OtherType>
  MatrixBlockData(std::shared_ptr<MatrixBlockData<Other, OtherType>> other)
      : MatrixBlockData(other->blockSize(), other->nbBlocksRows(),
                        other->nbBlocksCols(), other->x(), other->y(),
                        other->matrixWidth(), other->matrixHeight(),
                        other->get(), other->fullMatrix()) {}

  size_t blockSize() const { return blockSize_; }
  size_t nbBlocksRows() const { return nbBlocksRows_; }
  size_t nbBlocksCols() const { return nbBlocksCols_; }

  size_t x() const { return x_; }
  size_t y() const { return y_; }
  void x(size_t x) { this->x_ = x; }
  void y(size_t y) { this->y_ = y; }

  size_t matrixWidth() const { return matrixWidth_; }
  size_t matrixHeight() const { return matrixHeight_; }

  T *fullMatrix() { return fullMatrix_; }
  T *get() { return ptr_; }

  // helper function (warn: we use i and j here and not x and y so it's
  // inverted)
  T &at(size_t i, size_t j) { return ptr_[i * matrixWidth_ + j]; }
  T &fullMatrixAt(size_t i, size_t j) {
    return fullMatrix_[i * matrixWidth_ + j];
  }

private:
  size_t blockSize_ = 0;
  size_t nbBlocksRows_ = 0;
  size_t nbBlocksCols_ = 0;
  size_t x_ = 0;
  size_t y_ = 0;
  size_t matrixWidth_ = 0;
  size_t matrixHeight_ = 0;
  T *ptr_ = nullptr;
  T *fullMatrix_ = nullptr;
};

#endif
