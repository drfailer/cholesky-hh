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

#ifndef MATRIX_BLOCK_DATA_H
#define MATRIX_BLOCK_DATA_H

#include "block_types.h"
#include <cmath>
#include <cstddef>
#include <memory>
#include <hedgehog/hedgehog.h>

template <typename T, BlockTypes BlockType>
class MatrixBlockData {
 public:
  MatrixBlockData(size_t width, size_t height, size_t nbBlocksRows, size_t nbBlocksCols, size_t x,
                  size_t y, size_t matrixWidth, size_t matrixHeight, T *ptr, T *fullMatrix)
          : width_(width), height_(height), nbBlocksRows_(nbBlocksRows),
            nbBlocksCols_(nbBlocksCols), x_(x), y_(y), matrixWidth_(matrixWidth),
            matrixHeight_(matrixHeight), ptr_(ptr), fullMatrix_(fullMatrix) {}

  template <BlockTypes OtherType>
  explicit MatrixBlockData(std::shared_ptr<MatrixBlockData<T, OtherType>> &other)
          : MatrixBlockData(other->width(), other->height(), other->nbBlocksRows(),
                            other->nbBlocksCols(), other->x(), other->y(), other->matrixWidth(),
                            other->matrixHeight(), other->get(), other->fullMatrix()) {
    rank_ = other->rank();
  }

  template <BlockTypes OtherType>
  explicit MatrixBlockData(std::shared_ptr<MatrixBlockData<T, OtherType>> &&other)
          : MatrixBlockData(other->width(), other->height(), other->nbBlocksRows(),
                            other->nbBlocksCols(), other->x(), other->y(), other->matrixWidth(),
                            other->matrixHeight(), other->get(), other->fullMatrix()) {
    rank_ = other->rank();
  }

  template <BlockTypes OtherType>
  explicit MatrixBlockData(MatrixBlockData<T, OtherType> &&other)
          : MatrixBlockData(other.width(), other.height(), other.nbBlocksRows(),
                            other.nbBlocksCols(), other.x(), other.y(), other.matrixWidth(),
                            other.matrixHeight(), other.get(), other.fullMatrix()) {
    rank_ = other.rank();
  }

  [[nodiscard]] size_t width() const { return width_; }
  [[nodiscard]] size_t height() const { return height_; }

  [[nodiscard]] size_t nbBlocksRows() const { return nbBlocksRows_; }
  [[nodiscard]] size_t nbBlocksCols() const { return nbBlocksCols_; }

  [[nodiscard]] size_t x() const { return x_; }
  [[nodiscard]] size_t y() const { return y_; }
  void x(size_t x) { this->x_ = x; }
  void y(size_t y) { this->y_ = y; }

  [[nodiscard]] size_t matrixWidth() const { return matrixWidth_; }
  [[nodiscard]] size_t matrixHeight() const { return matrixHeight_; }

  [[nodiscard]] size_t rank() const { return rank_; }
  size_t incRank() { return ++rank_; }
  size_t decRank() { return --rank_; }
  void rank(size_t rank) { rank_ = rank; }

  // helper functions to simplify tests
  [[nodiscard]] bool isProcessed() const { return rank_ > x_; }
  [[nodiscard]] bool isReady() const { return rank_ == x_; }
  [[nodiscard]] bool isUpdateable(size_t rank) const { return rank_ == rank - 1; }
  [[nodiscard]] bool isDiag() const { return y_ == x_; }

  [[nodiscard]] size_t idx() const { return y_ * nbBlocksCols_ + x_; }
  [[nodiscard]] size_t diagIdx() const { return x_ * nbBlocksCols_ + x_; }

  T *fullMatrix() { return fullMatrix_; }
  T *get() { return ptr_; }

  // helper function (warn: we use i and j here and not x and y so it's
  // inverted)
  T &at(size_t i, size_t j) { return ptr_[i * matrixWidth_ + j]; }
  T &fullMatrixAt(size_t i, size_t j) { return fullMatrix_[i * matrixWidth_ + j]; }

  friend std::ostream &
  operator<<(std::ostream &os, const MatrixBlockData<T, BlockType> &block) {
    os << block.x_ << " " << block.y_ << " (" << block.rank_ << ")";
    return os;
  }

  friend std::ostream &
  operator<<(std::ostream &os, const std::shared_ptr<MatrixBlockData<T, BlockType>> &block) {
    if (block) {
      os << *block;
    } else {
      os << "nullptr";
    }
    return os;
  }

 private:
  size_t width_ = 0;
  size_t height_ = 0;
  size_t nbBlocksRows_ = 0;
  size_t nbBlocksCols_ = 0;
  size_t x_ = 0;
  size_t y_ = 0;
  size_t matrixWidth_ = 0;
  size_t matrixHeight_ = 0;
  size_t rank_ = 0;
//  bool isReady_ = false;
  T *ptr_ = nullptr;
  T *fullMatrix_ = nullptr;
};

#endif
