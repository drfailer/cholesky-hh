#ifndef MATRIX_DATA_H
#define MATRIX_DATA_H
#include <cstddef>
#include <hedgehog/hedgehog.h>

template <typename T> class MatrixData {
public:
  MatrixData(size_t width, size_t height, T *ptr):
    width_(width), height_(height), ptr_(ptr) {}

  size_t width() const { return width_; }
  size_t height() const { return height_; }

  void width(size_t width) { this->width_ = width; }
  void height(size_t height) { this->height_ = height; }

  T *get() { return ptr_; }

private:
  T *ptr_ = nullptr;
  size_t width_ = 0;
  size_t height_ = 0;
};

#endif
