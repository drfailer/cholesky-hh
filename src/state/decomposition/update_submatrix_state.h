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

#ifndef CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_H
#define CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_H

#include <vector>
#include <list>
#include "hedgehog/hedgehog/hedgehog.h"
#include "../../data/matrix_block_data.h"
#include "../../task/decomposition/update_submatrix_block_task.h"

#define USMStateInNb 3
#define USMStateIn                 \
  MatrixBlockData<T, MatrixBlock>, \
  MatrixBlockData<T, Column>,      \
  MatrixBlockData<T, Updated>
#define USMStateOut UpdateSubmatrixBlockInputType<T>

template <typename T>
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
      processPending();
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

      pending_.emplace_back(TripleIndex(col1Idx, col2Idx, updatedIdx));
    }
    processPending();
  }

  /* Updated ******************************************************************/

  /// @brief Receives updated blocks from decompose state
  void execute(std::shared_ptr<MatrixBlockData<T, Updated>>) override {
    processPending();
  }

  /* isDone *******************************************************************/

  [[nodiscard]] bool isDone() const {
    return blocks_.size() && blocks_.back() && blocks_.back()->isProcessed();
  }

 private:

  /* Types ********************************************************************/

  struct TripleIndex {
    TripleIndex(size_t col1Idx, size_t col2Idx, size_t updateIdx) :
            col1Idx(col1Idx), col2Idx(col2Idx), updateIdx(updateIdx) {}
    size_t col1Idx;
    size_t col2Idx;
    size_t updateIdx;
  };

  /* Variables ****************************************************************/

  std::vector<std::shared_ptr<MatrixBlockData<T, MatrixBlock>>> blocks_ = {};
  std::list<TripleIndex> pending_ = {};
  size_t blocksTtl_ = 0;
  size_t nbBlocksCols_ = 0;

  /* Process function *********************************************************/

  void processPending() {
    auto it = pending_.begin();

    while (it != pending_.end()) {
      auto col1 = blocks_[it->col1Idx];
      auto col2 = blocks_[it->col2Idx];
      auto updated = blocks_[it->updateIdx];
      bool col1Processed = col1 && col1->isProcessed();
      bool col2Processed = col2 && col2->isProcessed();
      bool updatedReady = col1 && updated && updated->isUpdateable(col1->rank());

      if (col1Processed && col2Processed && updatedReady) {
        this->addResult(std::make_shared<TripleBlockData<T>>(col1, col2, updated));
        it = pending_.erase(it);
      } else {
        it++;
      }
    }
  }
};

#endif //CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_H
