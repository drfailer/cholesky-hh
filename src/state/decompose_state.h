#ifndef DECOMPOSE_STATE_H
#define DECOMPOSE_STATE_H

#include "../data/matrix_block_data.h"
#include "../task/compute_column_block_task.h"
#include <hedgehog/hedgehog.h>
#include <vector>

#define DStateInNb 3
#define DStateIn MatrixBlockData<T, Block>, MatrixBlockData<T, Result>, MatrixBlockData<T, Updated>
#define DStateOut MatrixBlockData<T, Diagonal>, CCBTaskInputType<T>, MatrixBlockData<T, Result>

template<typename T>
class DecomposeState : public hh::AbstractState<DStateInNb, DStateIn, DStateOut > {
 public:
  explicit DecomposeState() : hh::AbstractState<DStateInNb, DStateIn, DStateOut >() {}

  /// @brief Receives the blocks from the SplitMatrix task.
  /// todo: we can optimize here by sending the first diagonal element early.
  void execute(std::shared_ptr<MatrixBlockData<T, Block>> block) override {
    // init array and blockttl on first call
    // todo: this could be done in the constructor
    if (blocks_.size() == 0) {
      init(block->nbBlocksRows(), block->nbBlocksCols());
      blocksTtl_ = (nbBlocksCols_ * (nbBlocksCols_ + 1)) / 2;
    }

    --blocksTtl_;
    blocks_[block->y() * nbBlocksRows_ + block->x()] = block;
    if (blocksTtl_ == 0) {
      size_t n = nbBlocksCols_ - currentColumnIdx_ - 1;
      updatedBlocksCounter_ = (n * (n + 1)) / 2;
      this->addResult(std::make_shared<MatrixBlockData<T, Diagonal>>(
              blocks_[currentColumnIdx_ * nbBlocksCols_ + currentColumnIdx_]));
    }
  }

  /// @brief Receives the updated blocks from the UpdateBlocks task. When all the blocks are
  /// updated, we start a new column.
  /// todo: we can optimize here by sending the next diagonal element early.
  void execute(std::shared_ptr<MatrixBlockData<T, Updated>>) override {
    --updatedBlocksCounter_;

    if (updatedBlocksCounter_ == 0) {
      size_t n = nbBlocksCols_ - currentColumnIdx_ - 1;
      updatedBlocksCounter_ = (n * (n + 1)) / 2;
      this->addResult(std::make_shared<MatrixBlockData<T, Diagonal>>(
              blocks_[currentColumnIdx_ * nbBlocksCols_ + currentColumnIdx_]));
    }
  }

  /// @brief Receives the result blocks from the ComputeDiagonal task and the ComputeColumn block
  /// task. This blocks are the output of the graph. When a new diagonal block is received, we can
  /// start processing the blocks on the column beneath.
  void execute(std::shared_ptr<MatrixBlockData<T, Result>> block) override {
    if (block->x() == block->y()) {
      currentDiagonalBlock_ = std::make_shared<MatrixBlockData<T, Diagonal>>(block);
      for (size_t i = block->y() + 1; i < nbBlocksRows_; ++i) {
        auto blk = blocks_[i * nbBlocksCols_ + block->x()];
        this->addResult(std::make_shared<CCBTaskInputType<T>>(currentDiagonalBlock_, blk));
      }
      ++currentColumnIdx_;
    }
    this->addResult(block); // return result block
  }

  [[nodiscard]] bool isDone() const {
    return currentDiagonalBlock_ && currentColumnIdx_ >= nbBlocksCols_;
  }

 private:
  std::vector<std::shared_ptr<MatrixBlockData<T, Block>>> blocks_ = {};
  std::shared_ptr<MatrixBlockData<T, Diagonal>> currentDiagonalBlock_ = nullptr;
  size_t nbBlocksRows_ = 0;
  size_t nbBlocksCols_ = 0;
  size_t currentColumnIdx_ = 0;
  size_t updatedBlocksCounter_ = 0;
  size_t blocksTtl_ = 0;

  void init(size_t nbBlocksRows, size_t nbBlocksCols) {
    nbBlocksRows_ = nbBlocksRows;
    nbBlocksCols_ = nbBlocksCols;
    blocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, Block>>>(
            nbBlocksRows_ * nbBlocksCols_, nullptr);
  }
};

#endif
