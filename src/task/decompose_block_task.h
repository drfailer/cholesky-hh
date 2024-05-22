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

    for (size_t i = 0; i < LBlock->blockSize(); ++i) {
      for (size_t j = 0; j <= i; ++j) {
        T sum = LBlock->at(i, j); // we already have a partial sum here
        for (size_t k = 0; k < j; k++) {
          sum += (LBlock->at(i, k) * LBlock->at(j, k));
        }

        if (j == i) {
          LBlock->at(i, j) = sqrt(ABlock->at(j, j) - sum);
        } else {
          LBlock->at(i, j) = (ABlock->at(i, j) - sum) / LBlock->at(j, j);
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
