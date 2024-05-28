#ifndef DECOMPOSE_BLOCK_TASK_H
#define DECOMPOSE_BLOCK_TASK_H
#include "../data/matrix_block_data.h"
#include <hedgehog/hedgehog.h>

#define DBTaskInNb 1
#define DBTaskIn MatrixBlockData<T, Block>
#define DBTaskOut MatrixBlockData<T, Result>

template <typename T>
class DecomposeBlockTask
    : public hh::AbstractTask<DBTaskInNb, DBTaskIn, DBTaskOut> {
public:
  DecomposeBlockTask(size_t nbThreads)
      : hh::AbstractTask<DBTaskInNb, DBTaskIn, DBTaskOut>(
            "Decompose block task", nbThreads) {}

  void execute(std::shared_ptr<MatrixBlockData<T, Block>> block) override {
    size_t iBegin = block->y() * block->blockSize();
    size_t iEnd = std::min(iBegin + block->blockSize(), block->matrixHeight());
    size_t jBegin = block->x() * block->blockSize();
    size_t jEnd = std::min(jBegin + block->blockSize(), block->matrixWidth());

    for (size_t i = iBegin; i < iEnd; ++i) {
      for (size_t j = jBegin; j < jEnd; ++j) {
        if (block->x() == block->y() && j > i) {
          break; // skip upper elements on the diagonal blocks
        }
        T sum = 0;
        for (size_t k = 0; k < j; k++) {
          sum += (block->fullMatrixAt(i, k) * block->fullMatrixAt(j, k));
        }

        if (j == i) {
          block->fullMatrixAt(i, j) = sqrt(block->fullMatrixAt(j, j) - sum);
        } else {
          block->fullMatrixAt(i, j) =
              (block->fullMatrixAt(i, j) - sum) / block->fullMatrixAt(j, j);
        }
      }
    }
    this->addResult(std::make_shared<MatrixBlockData<T, Result>>(block));
  }

  std::shared_ptr<hh::AbstractTask<DBTaskInNb, DBTaskIn, DBTaskOut>>
  copy() override {
    return std::make_shared<DecomposeBlockTask<T>>(this->numberThreads());
  }
};

#endif
