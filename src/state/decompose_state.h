#ifndef DECOMPOSE_STATE_H
#define DECOMPOSE_STATE_H

#include "../data/matrix_block_data.h"
#include "../task/compute_column_block_task.h"
#include <hedgehog/hedgehog.h>
#include <vector>

#define DStateInNb 4
#define DStateIn MatrixBlockData<T, Block>, MatrixBlockData<T, Diagonal>, MatrixBlockData<T, Column>, MatrixBlockData<T, Updated>
#define DStateOut MatrixBlockData<T, Diagonal>, MatrixBlockData<T, Column>, CCBTaskInputType<T>, MatrixBlockData<T, Updated>, MatrixBlockData<T, Result>

template<typename T>
class DecomposeState : public hh::AbstractState<DStateInNb, DStateIn, DStateOut > {
 public:
  explicit DecomposeState() : hh::AbstractState<DStateInNb, DStateIn, DStateOut >() {}

  /* Blocks *******************************************************************/

  /// @brief Receives the blocks from the SplitMatrix task.
  void execute(std::shared_ptr<MatrixBlockData<T, Block>> block) override {
    // init array and blockttl on first call
    if (blocks_.size() == 0) {
      init(block->nbBlocksRows(), block->nbBlocksCols());
      blocksTtl_ = (nbBlocksCols_ * (nbBlocksCols_ + 1)) / 2;
    }

    blocks_.at(block->y() * nbBlocksRows_ + block->x()) = block;
    --blocksTtl_;

    // todo
    if (block->isReady()) {
      if (block->isDiag()) {
        this->addResult(std::make_shared<MatrixBlockData<T, Diagonal>>(block));
      } else if (blocks_.at(block->diagIdx())->isProcessed()) {
        this->addResult(std::make_shared<CCBTaskInputType<T>>(
              std::make_shared<MatrixBlockData<T, Diagonal>>(blocks_.at(block->diagIdx())),
              block));
      } else {
        // the block will be treated when the diag element is received
      }
    }
  }

  /* Diagonal *****************************************************************/

  /// @brief Receives blocks from the ComputeColumn task
  void execute(std::shared_ptr<MatrixBlockData<T, Diagonal>> diag) override {
    blocks_.at(diag->idx())->incRank();
    diag->incRank();

    for (size_t i = diag->y() + 1; i < nbBlocksCols_; ++i) {
      auto block = blocks_.at(i * nbBlocksCols_ + diag->x());
      if (block && block->isReady()) {
        this->addResult(std::make_shared<CCBTaskInputType<T>>(diag, block));
      }
    }
    // the block is done so we can output the result
    this->addResult(std::make_shared<MatrixBlockData<T, Result>>(diag));
  }

  /* Column *******************************************************************/

  /// @brief Receives blocks from the ComputeDiagonalBlock task
  void execute(std::shared_ptr<MatrixBlockData<T, Column>> col) override {
    blocks_.at(col->idx())->incRank();
    col->incRank();

    // send block to the update state
    this->addResult(std::make_shared<MatrixBlockData<T, Column>>(blocks_.at(col->idx())));

    // the block is done so we can output the result
    this->addResult(std::make_shared<MatrixBlockData<T, Result>>(col));
  }

  /* Updated ******************************************************************/

  /// @brief Receives the updated blocks from the UpdateBlocks task. When all the blocks are
  /// updated, we start a new column.
  void execute(std::shared_ptr<MatrixBlockData<T, Updated>> block) override {
    blocks_.at(block->idx())->incRank();
    block->incRank();

    // todo
    if (block->isReady()) {
      if (block->isDiag()) {
        this->addResult(std::make_shared<MatrixBlockData<T, Diagonal>>(block));
      } else if (blocks_.at(block->diagIdx())->isProcessed()) {
        this->addResult(std::make_shared<CCBTaskInputType<T>>(
              std::make_shared<MatrixBlockData<T, Diagonal>>(blocks_.at(block->diagIdx())),
              blocks_.at(block->idx())));
      } else {
        // the block will be treated when the diag element is received
      }
    }

    // we may have to notify the update state
    this->addResult(block);
  }

  /* idDone ******************************************************************/

  [[nodiscard]] bool isDone() const {
    return blocks_.size() && blocks_.back() && blocks_.back()->isProcessed();
  }

 private:
  std::vector<std::shared_ptr<MatrixBlockData<T, Block>>> blocks_ = {};
  std::vector<std::shared_ptr<MatrixBlockData<T, Block>>> columnBlocks_ = {};
  size_t nbBlocksRows_ = 0;
  size_t nbBlocksCols_ = 0;
  size_t blocksTtl_ = 0;
  size_t columnBlockTtl_ = 0;

  /* helper functions *********************************************************/

  void init(size_t nbBlocksRows, size_t nbBlocksCols) {
    nbBlocksRows_ = nbBlocksRows;
    nbBlocksCols_ = nbBlocksCols;
    blocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, Block>>>(
            nbBlocksRows_ * nbBlocksCols_, nullptr);
  }
};

#endif
