// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
// nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
// source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
// WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
// THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
// are solely responsible for determining the appropriateness of using and distributing the software and you assume
// all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
// with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.

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
