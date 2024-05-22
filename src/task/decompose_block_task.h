#ifndef DECOMPOSE_BLOCK_TASK_H
#define DECOMPOSE_BLOCK_TASK_H
#include "../data/matrix_block_data.h"
#include <hedgehog/hedgehog.h>

template <typename T>
using DBInType = std::pair<std::shared_ptr<MatrixBlockData<T, Block>>,
                           std::shared_ptr<MatrixBlockData<T, Block>>>;

#define DBTaskInNb 1
#define DBTaskIn DBInType<T>
#define DBTaskOut MatrixBlockData<T, Result>

template <typename T>
class DecomposeBlockTask
    : public hh::AbstractTask<DBTaskInNb, DBTaskIn, DBTaskOut> {
public:
  DecomposeBlockTask(size_t nbThreads)
      : hh::AbstractTask<DBTaskInNb, DBTaskIn, DBTaskOut>(
            "Decompose block task", nbThreads) {}

  void execute(std::shared_ptr<DBInType<T>> blocks) override {
    auto ABlock = blocks->first();
    auto LBlock = blocks->second();
    size_t iEnd = LBlock->y() +
                  std::min(LBlock->blockSize(), LBlock->height() - LBlock->y());
    size_t jEnd = LBlock->x() +
                  std::min(LBlock->blockSize(), LBlock->width() - LBlock->x());

    for (size_t i = LBlock->y(); i < iEnd; ++i) {
      for (size_t j = LBlock->x(); j < jEnd; ++j) {
        if (LBlock->x() == LBlock->y() && j > i) {
          break; // skip upper elements on the diagnal blocks
        }
        T sum = 0;
        for (size_t k = 0; k < j; k++) {
          sum += (LBlock->fullMatrixAt(i, k) * LBlock->fullMatrixAt(j, k));
        }

        if (j == i) {
          LBlock->fullMatrixAt(i, j) = sqrt(ABlock->fullMatrixAt(j, j) - sum);
        } else {
          LBlock->fullMatrixAt(i, j) =
              (ABlock->fullMatrixAt(i, j) - sum) / LBlock->fullMatrixAt(j, j);
        }
      }
    }
    this->addResult(std::make_shared<MatrixBlockData<T, Result>>(LBlock));
  }

  std::shared_ptr<hh::AbstractTask<DBTaskInNb, DBTaskIn, DBTaskOut>>
  copy() override {
    return std::make_shared<DecomposeBlockTask<T>>(this->numberThreads());
  }
};

#endif
