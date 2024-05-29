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

  /// @brief Receives the blocks from the SplitMatrix task, saves them and send some of them to the
  /// ComputeColumnBlocks task if possible (if the current diagonal block is already processed).
  void execute(std::shared_ptr<MatrixBlockData<T, Block>> block) override {
    std::cout << "decompose state => Block" << std::endl;
    if (blocks_.size() == 0) {
      init(block->nbBlocksRows(), block->nbBlocksCols());
    }

    blocks_[block->y() * nbBlocksRows_ + block->x()] = block;
    if (block->x() == block->rank()) {
      if (block->y() == block->rank()) {
        this->addResult(std::make_shared<MatrixBlockData<T, Diagonal>>(block));
      } else if (currentDiagonalBlock_ != nullptr && currentDiagonalBlock_->x() == block->x()) {
        this->addResult(std::make_shared<CCBTaskInputType<T>>(currentDiagonalBlock_, block));
      }
    }
  }

  /// @brief Receives the updated blocks from the UpdateBlocks task. The blocks that are ready to be
  /// processed are sent either to the ComputeDiagonal task (for a diagonal block) or to the
  /// ComputeColumn task (otherwise).
  /// We keep track of the modifications on the blocks using the rank. For instance, if a block as a
  /// rank of 2, this means that it has been updated 2 times (by the UpdateBlocks task). If this
  /// block is on the second column, it is ready to be processed by the ComputeDiagnal/Column task
  /// (the diagonal should always be processed first).
  void execute(std::shared_ptr<MatrixBlockData<T, Updated>> block) override {
    block->incRank();
    blocks_[block->y() * nbBlocksRows_ + block->x()]->incRank();
    std::cout << "decompose state => Updated" << std::endl;
    std::cout << "Updated: " << block->x() << ", " << block->y() << " - " << block->rank()
              << std::endl;

    if (block->x() == block->rank()) {
      if (block->y() == block->rank()) {
        std::cout << "Updated => new diag: " << block->x() << std::endl;
        this->addResult(std::make_shared<MatrixBlockData<T, Diagonal>>(block));
      } else if (currentDiagonalBlock_ && currentDiagonalBlock_->x() == block->x()) {
        std::cout << "sending: " << block->x() << ", " << block->y() << std::endl;
        this->addResult(
                std::make_shared<CCBTaskInputType<T>>(currentDiagonalBlock_,
                                                      std::make_shared<MatrixBlockData<T, Block>>(
                                                              block)));
      }
    }
  }

  /// @brief Receives the result blocks from the ComputeDiagonal task and the ComputeColumn block
  /// task. This blocks are the output of the graph. When a new diagonal block is received, we can
  /// start processing the blocks on the column beneath (if they have been updated, ie there rank is
  /// equal the current column index, otherwise they will be sent to process by the previous
  /// function on receive).
  /// note: Here we make sure that the new diagonal element is processed before we start to process
  /// elements on the column. This is in this function / that we start a new step (ie we start
  /// processing a new column).
  void execute(std::shared_ptr<MatrixBlockData<T, Result>> block) override {
    std::cout << "decompose state => Result" << std::endl;
    std::cout << "Result: " << block->x() << ", " << block->y() << " - " << block->rank()
              << std::endl;

    if (block->x() == block->y()) {
      std::cout << "decompose state => diag ready" << std::endl;
      currentDiagonalBlock_ = std::make_shared<MatrixBlockData<T, Diagonal>>(block);

      // the diagonal element is ready, so we can send all the elements on the column beneath if
      // there updated is finished (we check the rank)
      for (size_t i = block->y() + 1; i < nbBlocksRows_; ++i) {
        auto blk = blocks_[i * nbBlocksRows_ + block->x()];
        if (blk && blk->rank() == currentDiagonalBlock_->x()) {
          std::cout << "sending: " << blk->x() << ", " << blk->y() << std::endl;
          this->addResult(std::make_shared<CCBTaskInputType<T>>(currentDiagonalBlock_, blk));
        }
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

  void init(size_t nbBlocksRows, size_t nbBlocksCols) {
    nbBlocksRows_ = nbBlocksRows;
    nbBlocksCols_ = nbBlocksCols;
    blocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, Block>>>(
            nbBlocksRows_ * nbBlocksCols_, nullptr);
  }
};

#endif
