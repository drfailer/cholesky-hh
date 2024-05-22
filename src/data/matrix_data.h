#ifndef MATRIX_DATA_H
#define MATRIX_DATA_H
#include <cstddef>
#include <hedgehog/hedgehog.h>

template <typename T> class MatrixData {
public:
  MatrixData(size_t width, size_t height, size_t blockSize, T *ptr)
      : width_(width), height_(height), blockSize_(blockSize),
        nbBlocksRows_(std::ceil(height / blockSize) +
                      (height % blockSize == 0 ? 0 : 1)),
        nbBlocksCols_(std::ceil(width / blockSize) +
                      (width % blockSize == 0 ? 0 : 1)),
        ptr_(ptr) {}

  size_t blockSize() const { return blockSize_; }
  size_t nbBlocksRows() const { return nbBlocksRows_; }
  size_t nbBlocksCols() const { return nbBlocksCols_; }

  size_t width() const { return width_; }
  size_t height() const { return height_; }

  void width(size_t width) { this->width_ = width; }
  void height(size_t height) { this->height_ = height; }

  T *get() { return ptr_; }

  friend std::ostream& operator<<(std::ostream& os, const MatrixData& matrix) {
    for (size_t i = 0; i < matrix.height(); ++i) {
      for (size_t j = 0; j < matrix.width(); ++j) {
        os << matrix.ptr_[i * matrix.width() + j] << " ";
      }
      os << std::endl;
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
