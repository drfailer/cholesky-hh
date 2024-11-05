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

#ifndef DECOMPOSE_STATE_H
#define DECOMPOSE_STATE_H

#include "../../data/matrix_block_data.h"
#include "../../task/decomposition/compute_column_block_task.h"
#include "hedgehog/hedgehog/hedgehog.h"
#include <vector>

#define DStateInNb 4
#define DStateIn                   \
  MatrixBlockData<T, MatrixBlock>, \
  MatrixBlockData<T, Diagonal>,    \
  MatrixBlockData<T, Column>,      \
  MatrixBlockData<T, Updated>
#define DStateOut                  \
  MatrixBlockData<T, MatrixBlock>, \
  MatrixBlockData<T, Diagonal>,    \
  MatrixBlockData<T, Column>,      \
  CCBTaskInputType<T>,             \
  MatrixBlockData<T, Updated>,     \
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

    this->addResult(block); // send the block to the update state
    tryProcessBlock(block); // start early computation if possible
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

    tryProcessBlock(blocks_[block->idx()]);
    this->addResult(block); // we may have to notify the update state
  }

  /* idDone ******************************************************************/

  [[nodiscard]] bool isDone() const {
    return !blocks_.empty() && blocks_.back() && blocks_.back()->isProcessed();
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

  void tryProcessBlock(std::shared_ptr<MatrixBlockData<T, MatrixBlock>> &block) {
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
};

#endif
