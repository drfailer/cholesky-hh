#ifndef CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_H
#define CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_H

#include <vector>
#include <list>
#include "hedgehog/hedgehog/hedgehog.h"
#include "../../data/matrix_block_data.h"
#include "../../task/decomposition/update_submatrix_block_task.h"

#define USMStateInNb 3
#define USMStateIn MatrixBlockData<T, MatrixBlock>, MatrixBlockData<T, Column>, MatrixBlockData<T, Updated>

#define USMStateOut UpdateSubmatrixBlockInputType<T>

template<typename T>
class UpdateSubMatrixState : public hh::AbstractState<USMStateInNb, USMStateIn, USMStateOut > {
 public:
  UpdateSubMatrixState() :
          hh::AbstractState<USMStateInNb, USMStateIn, USMStateOut >() {}

  /* Block ********************************************************************/

  /// @brief Receives the blocks from the SplitMatrix task and store them.
  void execute(std::shared_ptr<MatrixBlockData<T, MatrixBlock>> block) override {
    // special case for the first block received (may change as we can also give the information threw constructor)
    if (blocks_.size() == 0) {
      blocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, MatrixBlock>>>(
              block->nbBlocksCols() * block->nbBlocksRows(), nullptr);
      blocksTtl_ = (block->nbBlocksCols() * (block->nbBlocksCols() + 1)) / 2;
      nbBlocksCols_ = block->nbBlocksCols();
    }

    blocks_[block->y() * block->nbBlocksCols() + block->x()] = block;
    --blocksTtl_;

    if (blocksTtl_ == 0) {
      processPendings();
    }
  }

  /* Column *******************************************************************/

  /// @brief Receives result blocks from the ComputeColumn task. This blocks are used to update the
  /// rest of the matrix in the UpdateBlocks task.
  void execute(std::shared_ptr<MatrixBlockData<T, Column>> col) override {
    for (size_t i = col->y(); i < nbBlocksCols_; ++i) {
      size_t col1Idx = i * nbBlocksCols_ + col->x();
      size_t col2Idx = col->idx();
      size_t updatedIdx = i * nbBlocksCols_ + col->y();

      pendings_.emplace_back(TripleIndex(col1Idx, col2Idx, updatedIdx));
    }
    processPendings();
  }

  /* Updated ******************************************************************/

  /// @brief Receives updated blocks from decompose state
  void execute(std::shared_ptr<MatrixBlockData<T, Updated>>) override {
    processPendings();
  }

  /* isDone *******************************************************************/

  [[nodiscard]] bool isDone() const {
    return blocks_.size() && blocks_.back() && blocks_.back()->isProcessed();
  }

 private:
  struct TripleIndex {
    TripleIndex(size_t col1Idx, size_t col2Idx, size_t updateIdx):
      col1Idx(col1Idx), col2Idx(col2Idx), updateIdx(updateIdx) {}
    size_t col1Idx;
    size_t col2Idx;
    size_t updateIdx;
  };
  std::vector<std::shared_ptr<MatrixBlockData<T, MatrixBlock>>> blocks_ = {};
  std::list<TripleIndex> pendings_ = {};
  size_t blocksTtl_ = 0;
  size_t nbBlocksCols_ = 0;

  void processPendings() {
    auto it = pendings_.begin();

    while (it != pendings_.end()) {
      auto col1 = blocks_[it->col1Idx];
      auto col2 = blocks_[it->col2Idx];
      auto updated = blocks_[it->updateIdx];
      bool col1Processed = col1 && col1->isProcessed();
      bool col2Processed = col2 && col2->isProcessed();
      bool updatedReady = col1 && updated && updated->isUpdateable(col1->rank());

      if (col1Processed && col2Processed && updatedReady) {
        this->addResult(std::make_shared<TripleBlockData<T>>(col1, col2, updated));
        it = pendings_.erase(it);
      } else {
        it++;
      }
    }
  }
};

#endif //CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_H
