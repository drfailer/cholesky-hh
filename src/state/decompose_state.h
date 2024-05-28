#ifndef DECOMPOSE_STATE_H
#define DECOMPOSE_STATE_H

#include "../data/matrix_block_data.h"
#include "../task/compute_column_block_task.h"
#include <hedgehog/hedgehog.h>
#include <list>

#define DStateInNb 3
#define DStateIn MatrixBlockData<T, Block>, MatrixBlockData<T, Result>, MatrixBlockData<T, Updated>
#define DStateOut MatrixBlockData<T, Diagonal>, CCBTaskInputType<T>, MatrixBlockData<T, Result>

template<typename T>
class DecomposeState
        : public hh::AbstractState<DStateInNb, DStateIn, DStateOut > {
 public:
  explicit DecomposeState() : hh::AbstractState<DStateInNb, DStateIn, DStateOut >() {}

  void execute(std::shared_ptr<MatrixBlockData<T, Block>> block) override {
    blocks_.push_back(block);
    if (block->x() == colIndex_ && block->y() == colIndex_) {
      // we treat the first element
      nbElementsCol_ = block->nbBlocksRows() - colIndex_;
      currentDiagonalBlock_ = std::make_shared<MatrixBlockData<T, Diagonal>>(block);
      this->addResult(currentDiagonalBlock_);
    } else if (currentDiagonalBlock_ != nullptr && block->x() == colIndex_) {
      // this is the case where we still receive data from the split task and
      // we already have treated the first element.
      this->addResult(std::make_shared<CCBTaskInputType<T>>(currentDiagonalBlock_, block));
    }
  }

  void execute(std::shared_ptr<MatrixBlockData<T, Updated>>) override {
    // todo: this one will be used for optimization
  }

  void execute(std::shared_ptr<MatrixBlockData<T, Result>> block) override {
    // TODO: find a way to clearly detect when we can send the blocks instead of
    // just waiting for the diagonal element
    --nbElementsCol_;
    removeBlock(block);

    if (block->x() == block->y()) {
      // the diagonal element is ready so we can send all the elements beneath
      // on the column
      for (auto resultBlock: blocks_) {
        if (resultBlock->x() == block->x()) {
          this->addResult(
                  std::make_shared<CCBTaskInputType<T>>(currentDiagonalBlock_, resultBlock));
        }
      }
    } else if (nbElementsCol_ == 0) {
      // all the elements on the column are processed
      ++colIndex_;
      nbElementsCol_ = block->nbBlocksRows() - colIndex_;
      auto diagElt =
              std::find_if(blocks_.begin(), blocks_.end(), [this](auto rb) {
                return rb->x() == colIndex_ && rb->y() == colIndex_;
              });
      currentDiagonalBlock_ = std::make_shared<MatrixBlockData<T, Diagonal>>(*diagElt);
      this->addResult(currentDiagonalBlock_);
    }
    this->addResult(block); // result block
  }

  template<BlockTypes BlockType>
  void removeBlock(std::shared_ptr<MatrixBlockData<T, BlockType>> block) {
    auto pos = std::find_if(blocks_.begin(), blocks_.end(), [&](auto rb) {
      return rb->x() == block->x() && rb->y() == block->y();
    });
    if (pos != blocks_.end()) {
      blocks_.erase(pos);
    }
  }

  [[nodiscard]] bool isDone() const { return colIndex_ > 0 && blocks_.size() == 0; }

 private:
  // TODO: change the list to something more efficient for this problem (for
  // optimized version)
  std::list<std::shared_ptr<MatrixBlockData<T, Block>>> blocks_ = {};
  std::shared_ptr<MatrixBlockData<T, Diagonal>> currentDiagonalBlock_ = nullptr;
  size_t colIndex_ = 0;
  size_t nbElementsCol_ = 0;
};

#endif
