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

  void execute(std::shared_ptr<MatrixBlockData<T, Block>> block) override {
    std::cout << "decompose state => Block" << std::endl;
    // todo: should be done in the constructor
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

  void execute(std::shared_ptr<MatrixBlockData<T, Updated>> block) override {
    std::cout << "decompose state => Updated" << std::endl;
    std::cout << "Updated: " << block->x() << ", " << block->y() << " - " << block->rank() << std::endl;

    if (block->x() == block->rank()) {
      if (block->y() == block->rank()) {
        std::cout << "Updated => new diag: " << block->x() << std::endl;
        this->addResult(std::make_shared<MatrixBlockData<T, Diagonal>>(block));
      } else if (currentDiagonalBlock_ && currentDiagonalBlock_->x() == block->x()) {
        this->addResult(std::make_shared<CCBTaskInputType<T>>(currentDiagonalBlock_,
                                                              std::make_shared<MatrixBlockData<T, Block>>(
                                                                      block)));
      }
    }
  }

  void execute(std::shared_ptr<MatrixBlockData<T, Result>> block) override {
    std::cout << "decompose state => Result" << std::endl;
    std::cout << "Result: " << block->x() << ", " << block->y() << " - " << block->rank() << std::endl;

    if (block->x() == block->y()) {
      std::cout << "decompose state => diag ready" << std::endl;
      currentDiagonalBlock_ = std::make_shared<MatrixBlockData<T, Diagonal>>(block);
      // the diagonal element is ready so we can send all the elements beneath on the column
      for (size_t i = block->y() + 1; i < nbBlocksRows_; ++i) {
        auto blk = blocks_[i * nbBlocksRows_ + block->x()];
        if (blk && blk->rank() == currentDiagonalBlock_->x()) {
          std::cout << "sending: " << blk->x() << ", " << blk->y() << std::endl;
          this->addResult(std::make_shared<CCBTaskInputType<T>>(currentDiagonalBlock_, blk));
        }
      }
      ++colIdx_;
    }
    this->addResult(block); // return result block
  }

  [[nodiscard]] bool isDone() const {
    return currentDiagonalBlock_ && colIdx_ >= nbBlocksCols_;
  }

 private:
  std::vector<std::shared_ptr<MatrixBlockData<T, Block>>> blocks_ = {};
  std::shared_ptr<MatrixBlockData<T, Diagonal>> currentDiagonalBlock_ = nullptr;
  size_t nbBlocksRows_ = 0;
  size_t nbBlocksCols_ = 0;
  size_t colIdx_ = 0;

  void init(size_t nbBlocksRows, size_t nbBlocksCols) {
    nbBlocksRows_ = nbBlocksRows;
    nbBlocksCols_ = nbBlocksCols;
    blocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, Block>>>(
            nbBlocksRows_ * nbBlocksCols_, nullptr);
  }
};

#endif
