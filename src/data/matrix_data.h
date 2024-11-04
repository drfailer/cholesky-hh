#ifndef MATRIX_DATA_H
#define MATRIX_DATA_H

#include <cstddef>
#include <hedgehog/hedgehog.h>
#include <memory>
#include "matrix_types.h"

template<typename T, MatrixTypes MT = MatrixTypes::Matrix>
class MatrixData {
 public:
  MatrixData(size_t width, size_t height, size_t blockSize, T *ptr)
          : width_(width), height_(height), blockSize_(blockSize),
            nbBlocksRows_((size_t) std::ceil(height / blockSize) +
                          (height % blockSize == 0 ? 0 : 1)),
            nbBlocksCols_((size_t) std::ceil(width / blockSize) +
                          (width % blockSize == 0 ? 0 : 1)),
            ptr_(ptr) {}

  [[nodiscard]] size_t blockSize() const { return blockSize_; }

  [[nodiscard]] size_t nbBlocksRows() const { return nbBlocksRows_; }
  [[nodiscard]] size_t nbBlocksCols() const { return nbBlocksCols_; }

  [[nodiscard]] size_t width() const { return width_; }
  [[nodiscard]] size_t height() const { return height_; }

  [[nodiscard]] T at(size_t i, size_t j) const {
    return ptr_[i * width_ + j];
  }

  [[nodiscard]] T *get() { return ptr_; }

  void reset(const std::shared_ptr<MatrixData<T, MT>> &matrix) {
    size_t size = matrix->width_ * matrix->height_;

    for (size_t i = 0;  i < size; ++i) {
      this->ptr_[i] = matrix->get()[i];
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const std::shared_ptr<MatrixData<T, MT>> &matrix) {
    if (matrix == nullptr) {
      os << "nullptr" << std::endl;
    } else {
      for (size_t i = 0; i < matrix->height(); ++i) {
        for (size_t j = 0; j < matrix->width(); ++j) {
          os << matrix->ptr_[i * matrix->width() + j] << " ";
        }
        os << std::endl;
      }
    }
    return os;
  }

 private:
  size_t width_ = 0;
  size_t height_ = 0;
  size_t blockSize_ = 0;
  size_t nbBlocksRows_ = 0;
  size_t nbBlocksCols_ = 0;
  T *ptr_ = nullptr;
};

#endif
