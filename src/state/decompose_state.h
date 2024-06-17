#ifndef DECOMPOSE_STATE_H
#define DECOMPOSE_STATE_H

#include "../data/matrix_block_data.h"
#include "../task/compute_column_block_task.h"
#include <hedgehog/hedgehog.h>
#include <vector>

#define DStateInNb 4
#define DStateIn                   \
  MatrixBlockData<T, MatrixBlock>, \
  MatrixBlockData<T, Diagonal>,    \
  MatrixBlockData<T, Column>,      \
  MatrixBlockData<T, Updated>
#define DStateOut               \
  MatrixBlockData<T, Diagonal>, \
  MatrixBlockData<T, Column>,   \
  CCBTaskInputType<T>,          \
  MatrixBlockData<T, Updated>,  \
  MatrixBlockData<T, Decomposed>

template <typename T>
class DecomposeState : public hh::AbstractState<DStateInNb, DStateIn, DStateOut > {
 public:
  explicit DecomposeState() : hh::AbstractState<DStateInNb, DStateIn, DStateOut >() {}

  /* Blocks *******************************************************************/

  /// @brief Receives the blocks from the SplitMatrix task.
  void execute(std::shared_ptr<MatrixBlockData<T, MatrixBlock>> block) override {
    // init array and blockttl on first call
    if (blocks_.size() == 0) {
      init(block->nbBlocksRows(), block->nbBlocksCols());
      blocksTtl_ = (nbBlocksCols_ * (nbBlocksCols_ + 1)) / 2;
    }

    blocks_[block->y() * nbBlocksRows_ + block->x()] = block;
    --blocksTtl_;

    if (block->isReady()) {
      if (block->isDiag()) {
        this->addResult(std::make_shared<MatrixBlockData<T, Diagonal>>(block));
      } else if (blocks_[block->diagIdx()]->isProcessed()) {
        this->addResult(std::make_shared<CCBTaskInputType<T>>(
                std::make_shared<MatrixBlockData<T, Diagonal>>(blocks_[block->diagIdx()]),
                block));
      } // else the block will be treated when the diag element is received
    }
  }

  /* Diagonal *****************************************************************/

  /// @brief Receives blocks from the ComputeDiagonalBlock task
  void execute(std::shared_ptr<MatrixBlockData<T, Diagonal>> diag) override {
    blocks_[diag->idx()]->incRank();

    for (size_t i = diag->y() + 1; i < nbBlocksCols_; ++i) {
      auto block = blocks_[i * nbBlocksCols_ + diag->x()];
      if (block && block->isReady()) {
        this->addResult(std::make_shared<CCBTaskInputType<T>>(diag, block));
      }
    }
    // the block is done so we can output the result
    this->addResult(std::make_shared<MatrixBlockData<T, Decomposed>>(std::move(diag)));
  }

  /* Column *******************************************************************/

  /// @brief Receives blocks from the ComputeColumn task
  void execute(std::shared_ptr<MatrixBlockData<T, Column>> col) override {
    blocks_[col->idx()]->incRank();
    col->incRank();

    // send block to the update state
    this->addResult(col);

    // the block is done so we can output the result
    this->addResult(std::make_shared<MatrixBlockData<T, Decomposed>>(std::move(col)));
  }

  /* Updated ******************************************************************/

  /// @brief Receives the updated blocks from the UpdateBlocks task. When all the blocks are
  /// updated, we start a new column.
  void execute(std::shared_ptr<MatrixBlockData<T, Updated>> block) override {
    blocks_[block->idx()]->incRank();
    block->incRank();

    if (block->isReady()) {
      if (block->isDiag()) {
        this->addResult(std::make_shared<MatrixBlockData<T, Diagonal>>(block));
      } else if (blocks_[block->diagIdx()]->isProcessed()) {
        this->addResult(std::make_shared<CCBTaskInputType<T>>(
                std::make_shared<MatrixBlockData<T, Diagonal>>(blocks_[block->diagIdx()]),
                blocks_[block->idx()]));
      } // else the block will be treated when the diag element is received
    }

    // we may have to notify the update state
    this->addResult(block);
  }

  /* idDone ******************************************************************/

  [[nodiscard]] bool isDone() const {
    return blocks_.size() && blocks_.back() && blocks_.back()->isProcessed();
  }

 private:
  std::vector<std::shared_ptr<MatrixBlockData<T, MatrixBlock>>> blocks_ = {};
  size_t nbBlocksRows_ = 0;
  size_t nbBlocksCols_ = 0;
  size_t blocksTtl_ = 0;

  /* helper functions *********************************************************/

  void init(size_t nbBlocksRows, size_t nbBlocksCols) {
    nbBlocksRows_ = nbBlocksRows;
    nbBlocksCols_ = nbBlocksCols;
    blocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, MatrixBlock>>>(
            nbBlocksRows_ * nbBlocksCols_, nullptr);
  }
};

#endif
