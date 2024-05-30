#ifndef CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_H
#define CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_H

#include <hedgehog/hedgehog.h>
#include "../data/matrix_block_data.h"
#include "../task/update_submatrix_block_task.h"

#define USMStateInNb 2
#define USMStateIn MatrixBlockData<T, Block>, MatrixBlockData<T, Result>
#define USMStateOut UpdateSubmatrixBlockInputType<T>

template<typename T>
class UpdateSubMatrixState : public hh::AbstractState<USMStateInNb, USMStateIn, USMStateOut > {
 public:
  UpdateSubMatrixState() :
          hh::AbstractState<USMStateInNb, USMStateIn, USMStateOut >() {}

  /// @brief Receives the blocks from the SplitMatrix task and store them.
  void execute(std::shared_ptr<MatrixBlockData<T, Block>> block) override {
    // special case for the first block received (may change as we can also give the information threw constructor)
    if (blocks_.size() == 0) {
      blocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, Block>>>(
              block->nbBlocksCols() * block->nbBlocksRows(), nullptr);
      nbCols_ = block->nbBlocksCols() - 1;
      nbBlocksCurrentCol_ = nbCols_;
      blocksTtl_ = (block->nbBlocksCols() * (block->nbBlocksCols() + 1)) / 2;
    }
    --blocksTtl_;
    blocks_[block->y() * block->nbBlocksCols() + block->x()] = block;

    if (blocksTtl_ == 0) {
      for (auto unprocessedBlock : unprocessedColumnBlocks) {
        update(unprocessedBlock);
      }
    }
  }

  /// @brief Receives result blocks from the ComputeColumn task. This blocks are used to update the
  /// rest of the matrix in the UpdateBlocks task.
  void execute(std::shared_ptr<MatrixBlockData<T, Result>> block) override {
    if (blocksTtl_ == 0) { // security in the case where all the blocks are not received
      update(block);
    } else {
      unprocessedColumnBlocks.push_back(block);
    }
  }

  bool isDone() {
    return nbCols_ == 0 && nbBlocksCurrentCol_ == 0;
  }

 private:
  std::vector<std::shared_ptr<MatrixBlockData<T, Result>>> columnBlocks_ = {};
  std::vector<std::shared_ptr<MatrixBlockData<T, Result>>> unprocessedColumnBlocks = {};
  std::vector<std::shared_ptr<MatrixBlockData<T, Block>>> blocks_ = {};
  size_t nbCols_ = 1;
  size_t nbBlocksCurrentCol_ = 1;
  size_t blocksTtl_ = 0;

  void update(std::shared_ptr<MatrixBlockData<T, Result>> block) {
    // update the diagonal block
    size_t updateIdx = block->y() * block->nbBlocksCols() + block->y();
    this->addResult(std::make_shared<TripleBlockData<T>>(block, block, blocks_[updateIdx]));
    --nbBlocksCurrentCol_;
    // update the other blocks
    for (auto colBlock: columnBlocks_) {
      if (colBlock->y() > block->y()) {
        updateIdx = colBlock->y() * block->nbBlocksCols() + block->y();
        this->addResult(std::make_shared<TripleBlockData<T>>(colBlock, block, blocks_[updateIdx]));
      } else {
        updateIdx = block->y() * block->nbBlocksCols() + colBlock->y();
        this->addResult(std::make_shared<TripleBlockData<T>>(block, colBlock, blocks_[updateIdx]));
      }
    }
    columnBlocks_.push_back(block);
    if (nbBlocksCurrentCol_ == 0) {
      nbBlocksCurrentCol_ = --nbCols_;
      columnBlocks_.clear();
    }
  }
};

#endif //CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_H
