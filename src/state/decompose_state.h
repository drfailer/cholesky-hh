#ifndef DECOMPOSE_STATE_H
#define DECOMPOSE_STATE_H
#include "../data/matrix_block_data.h"
#include "../task/decompose_block_task.h"
#include <hedgehog/hedgehog.h>
#include <vector>

#define DStateInNb 2
#define DStateIn MatrixBlockData<T, Block>, MatrixBlockData<T, Result>
#define DStateOut MatrixBlockData<T, Block>, MatrixBlockData<T, Result>

template <typename T>
class DecomposeState
    : public hh::AbstractState<DStateInNb, DStateIn, DStateOut> {
public:
  DecomposeState() : hh::AbstractState<DStateInNb, DStateIn, DStateOut>() {}

  void execute(std::shared_ptr<MatrixBlockData<T, Block>> block) override {
    if (block->x() <= block->y()) {
      blocks_.push_back(block);
      if (block->x() == colIndex_ && block->y() == colIndex_) {
        // we treat the first element
        size_t nbBlockRows =
            std::ceil(block->matrixHeight() / block->blockSize()) +
            (block->matrixHeight() % block->blockSize() == 0 ? 0 : 1);
        nbElementCol_ = nbBlockRows - colIndex_;
        this->addResult(block);
      } else if (firstEltTreated_) {
        // this is the case where we still receive data from the split task and
        // we already have treated the first element.
        this->addResult(block);
      }
    }
  }

  void execute(std::shared_ptr<MatrixBlockData<T, Result>> block) override {
    // TODO: find a way to clearly detect when we can send the blocks instead of
    // just waiting for the diagonal element
    --nbElementCol_;
    removeBlock(block);

    if (block->x() == colIndex_ && block->y() == colIndex_) {
      firstEltTreated_ = true;
      // the diagonal element is ready so we can send all the elements beneath
      // on the column
      for (auto resultBlock : blocks_) {
        if (resultBlock->x() == colIndex_) {
          this->addResult(resultBlock);
        }
      }
    } else {
      // all the elements on the column are processed
      if (nbElementCol_ == 0) {
        ++colIndex_;
        nbElementCol_ = block->matrixWidth() - colIndex_;
        auto diagElt =
            std::find_if(blocks_.begin(), blocks_.end(), [this](auto rb) {
              return rb->x() == colIndex_ && rb->y() == colIndex_;
            });
        this->addResult(*diagElt);
      }
    }
    this->addResult(block);
  }

  template <BlockTypes BlockType>
  void removeBlock(std::shared_ptr<MatrixBlockData<T, BlockType>> block) {
    auto pos = std::find_if(blocks_.begin(), blocks_.end(), [&](auto rb) {
      return rb->x() == block->x() && rb->y() == block->y();
    });
    if (pos != blocks_.end()) {
      blocks_.erase(pos);
    }
  }

  bool isDone() const { return colIndex_ > 0 && blocks_.size() == 0; }

private:
  // TODO: change the vector to something more efficient for this problem (for
  // optimized version)
  std::vector<std::shared_ptr<MatrixBlockData<T, Block>>> blocks_ = {};
  size_t colIndex_ = 0;
  size_t nbElementCol_ = 0;
  bool firstEltTreated_ = false;
};

#endif
